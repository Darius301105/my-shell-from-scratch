#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
  char* line = NULL;
  size_t len = 0;

  while(1){
    fputs("my-shell-from-scratch> ", stdout);
    fflush(stdout);

    if(getline(&line, &len, stdin) == -1){
      break;
    }

    line[strcspn(line, "\n")] = '\0';

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
