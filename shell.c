#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

void handle_sigchld(int sig){
  (void)sig;

  int status;
  pid_t pid;

  while((pid = waitpid(-1, &status, WNOHANG)) > 0){
    printf("\n[done] PID %d finished\n", pid);
    fflush(stdout);
  }
}

void handle_sigint(int sig){
  (void)sig;
  printf("\n");
}


int main(){
  char* line = NULL;
  size_t len = 0;
  char cwd[1024];
  char prompt[1100];
  char *history[100];
  int history_cnt = 0;

  signal(SIGCHLD, handle_sigchld);
  signal(SIGINT, handle_sigint);

  while(1){
    int background = 0;
    if(getcwd(cwd, sizeof(cwd)) != NULL){
      snprintf(prompt, sizeof(prompt), "my-shell%s> ", cwd);
    }else{
      snprintf(prompt, sizeof(prompt), "my-shell> ");
    }
    fputs(prompt, stdout);
    fflush(stdout);

    if(getline(&line, &len, stdin) == -1){
      break;
    }

    line[strcspn(line, "\n")] = '\0';

    int len = strlen(line);

    if(len > 0 && line[len-1] == '&'){
      line[len-1] = '\0';
      background = 1;
    }


    if(strcmp(line, "") != 0){
      history[history_cnt] = strdup(line);
      history_cnt++;
    }

    if(strcmp(line, "exit\n") == 0 || strcmp(line, "exit") == 0){
      break;
    }

    char* redir_out = strchr(line, '>');

    if(redir_out != NULL){
      *redir_out = '\0';

      char *cmd_str = line;
      char *file_str = redir_out +1;

      while(*file_str == ' '){
        file_str++;
      }

      char* args[100];
      int i=0;

      char *token = strtok(cmd_str, " ");
      while(token != NULL){
        args[i++] = token;
        token = strtok(NULL, " ");
      }
      args[i] = NULL;

      if(args[0] == NULL || *file_str == '\0'){
        continue;
      }

      pid_t pid = fork();
      if(pid == 0){
        int fd = open(file_str, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd == -1){
          perror("open");
          exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);

        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
      }else if(pid > 0){
	wait(NULL);
      }else{
	perror("fork failed");
      }

      continue;
    }

    char *redir_in = strchr(line, '<');

    if(redir_in != NULL){
      *redir_in = '\0';

      char *cmd_str = line;
      char *file_str = redir_in +1;

      while(*file_str == ' '){
        file_str++;
      }

      char *args[100];
      int i=0;

      char *token = strtok(cmd_str, " ");
      while(token != NULL){
        args[i++] = token;
        token = strtok(NULL, " ");
      }
      args[i] = NULL;

      if(args[0] == NULL || *file_str == '\0'){
        continue;
      }

      pid_t pid = fork();
      if(pid == 0){
        int fd = open(file_str, O_RDONLY);
	if(fd == -1){
	  perror("exit");
          exit(1);
	}

	dup2(fd, STDIN_FILENO);
	close(fd);

	execvp(args[0], args);
	perror("execvp failed");
	exit(1);
      }else if(pid > 0){
        wait(NULL);
      }else{
        perror("fork failed");
      }

      continue;
    }

    int n_cmds = 1;
    for(int k=0; line[k] != '\0'; k++){
      if(line[k] == '|'){
        n_cmds++;
      }
    }

    char *cmds[100];
    int i=0;
    char *tok = strtok(line, "|");
    while(tok != NULL){
      cmds[i++] = tok;
      tok = strtok(NULL, "|");
    }

    if(n_cmds > 1){
      int fd[99][2];
      for(int p=0;p<n_cmds - 1;p++){
        if(pipe(fd[p]) == -1){
	  perror("pipe");
          exit(1);
        }
      }

      for(int i=0;i<n_cmds;i++){
        pid_t pid = fork();

        if(pid < 0){
          perror("fork");
          exit(1);
        }

        if(pid == 0){
          char *args[100];
          int j=0;
          char *tok2 = strtok(cmds[i], " \t\n");
          while(tok2 != NULL){
            args[j++] = tok2;
            tok2 = strtok(NULL, " \t\n");
          }
          args[j] = NULL;

          if(args[0] == NULL){
            exit(0);
          }

          if(i>0){
            if(dup2(fd[i-1][0], STDIN_FILENO) == -1){
              perror("dup2 stdin");
              exit(1);
            }
          }

          if(i<n_cmds - 1){
            if(dup2(fd[i][1], STDOUT_FILENO) == -1){
	      perror("dup2 stdout");
              exit(1);
            }
          }

          for(int k=0;k<n_cmds-1;k++){
            close(fd[k][0]);
            close(fd[k][1]);
          }

          execvp(args[0], args);
          perror("execvp");
          exit(1);
        }
      }

      for(int p=0;p<n_cmds-1;p++){
        close(fd[p][0]);
        close(fd[p][1]);
      }

      for(int i=0;i<n_cmds-1;i++){
        wait(NULL);
      }
      continue;
    }

    char *args[100];
    int a=0;

    char *token = strtok(line, " \t\n");
    while(token != NULL){
      args[a++] = token;
      token = strtok(NULL, " \t\n");
    }
    args[a] = NULL;

    for(int j=0; args[j]!=NULL;j++){
      if(args[j][0] == '$'){
        char *val = getenv(args[j]+1);

        if(val != NULL){
          args[j] = val;
        }else{
          args[j] = "";
        }
      }
    }

    if(args[0] == NULL){
      continue;
    }

    if(strcmp(args[0], "help") == 0){
      printf("Built-in commands:\n");
      printf(" exit  - exit the shell\n");
      printf(" cd    - change directory\n");
      printf(" pwd   - shows the current path\n");
      printf(" help  - message with all built-in commands\n");
      printf(" history - shows the command history\n");
      continue;
    }

    if(strcmp(args[0], "history") == 0){
      for(int i=0;i<history_cnt;i++){
        printf("%d %s\n", i+1, history[i]);
      }
      continue;
    }

    if(strcmp(args[0], "cd") == 0){
      if(args[1] == NULL){
        chdir(getenv("HOME"));
	continue;
      }else if(args[1] != NULL && strcmp(args[1], "~") == 0){
        chdir(getenv("HOME"));
        continue;
      }else{
        if(chdir(args[1]) == -1){
          perror("cd");
        }
      }
      continue;
    }


    pid_t pid = fork();
    if(pid == 0){
      execvp(args[0], args);
      perror("execvp failed");
      exit(1);
    }else if(pid>0){
       if(background == 0){
         wait(NULL);
       }else{
         printf("[background] PID %d started\n", pid);
       }
    }else{
       perror("fork failed");
    }
  }
   free(line);
   return 0;
}
