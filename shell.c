#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
  char* line = NULL;
  size_t len = 0;
  char cwd[1024];
  char prompt[1100];
  char *history[100];
  int history_cnt = 0;

  while(1){
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

    if(strcmp(line, "") != 0){
      history[history_cnt] = strdup(line);
      history_cnt++;
    }

    if(strcmp(line, "exit\n") == 0 || strcmp(line, "exit") == 0){
      break;
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
      perror("error");
      exit(1);
    }else if(pid>0){
       wait(NULL);
    }else{
       perror("fork failed");
    }
  }
   free(line);
   return 0;
}
