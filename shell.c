#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(){
  char* line = NULL;
  size_t len = 0;
  char cwd[1024];
  char prompt[1100];
  char *history[100];
  int history_cnt = 0;

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


    char* pipe_pos = strchr(line, '|');

    if(pipe_pos != NULL){
      *pipe_pos = '\0';

      char *cmd1_str = line;
      char *cmd2_str = pipe_pos +1;

      char* args1[100];
      int i=0;
      char *tok = strtok(cmd1_str, " ");
      while(tok != NULL){
        args1[i++] = tok;
        tok = strtok(NULL, " ");
      }
      args1[i] = NULL;

      char *args2[100];
      i=0;
      tok = strtok(cmd2_str, " ");
      while(tok != NULL){
        args2[i++] = tok;
        tok = strtok(NULL, " ");
      }
      args2[i] = NULL;
      continue;

      if(args1[0] == NULL || args2[0] == NULL){
        continue;
      }

      int fd[2];
      pipe(fd);

      if(pipe(fd) == -1){
        perror("pipe failed");
        continue;
      }

      pid_t pid1 = fork();
       if(pid1==0){
        dup2(fd[1], STDOUT_FILENO);

        close(fd[0]);
        close(fd[1]);

        execvp(args1[0], args1);
        perror("execvp cmd1 failed");
        exit(1);
      }else if(pid1 < 0){
        perror("fork1 failed");
        close(fd[0]);
        close(fd[1]);
        continue;
      }

      pid_t pid2 = fork();
      if(pid2 == 0){
        dup2(fd[1], STDOUT_FILENO);

        close(fd[0]);
        close(fd[1]);

        execvp(args2[0], args2);
        perror("execvp cmd2 failed");
        exit(1);
      }else if(pid2 < 0){
        perror("fork2 failed");
        close(fd[0]);
        close(fd[1]);
        wait(NULL);
        continue;
      }

      close(fd[0]);
      close(fd[1]);

      wait(NULL);
      wait(NULL);

      continue;
    }
    char *args[100];
    int i=0;

    char *token = strtok(line, " ");
    while(token != NULL){
      args[i++] = token;
      token = strtok(NULL, " ");
    }
    args[i] = NULL;

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
         printf("[background] process started\n");
       }
    }else{
       perror("fork failed");
    }
  }
   free(line);
   return 0;
}
