#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "utils.h"

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

void builtin_cd(char ** args){
  if(args[1] == NULL){      
    chdir(getenv("HOME"));
  }else if(strcmp(args[1], "~") == 0){      
    chdir(getenv("HOME"));
  }else{
    if(chdir(args[1]) == -1){
      perror("cd");
    }
  }
}

void builtin_history(char **history, int count){
  for(int i=0;i<count;i++){
    printf("%d %s\n", i+1, history[i]);
  }
}

void builtin_help(){
  printf("Built-in commands:\n");
  printf(" exit  - exit the shell\n");
  printf(" cd    - change directory\n");
  printf(" pwd   - shows the current path\n");
  printf(" help  - message with all built-in commands\n");
  printf(" history - shows the command history\n");
}

void execute_command(char **args, int background){
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

void execute_pipe(char **cmds, int n_cmds){
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
}

void execute_redir_out(char *cmd_str, char *file_str){
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
    return;
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
}

void execute_redir_in(char *cmd_str, char* file_str){
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
    return;
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
}







