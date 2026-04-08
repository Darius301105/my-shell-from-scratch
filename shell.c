#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#include "utils.h"


int main(){
  char* line = NULL;
  char cwd[1024];
  char prompt[1100];
  char *history[100];
  int history_cnt = 0;

  signal(SIGCHLD, handle_sigchld);
  signal(SIGINT, handle_sigint);

  while(1){
    int background = 0;
    if(getcwd(cwd, sizeof(cwd)) != NULL){
      snprintf(prompt, sizeof(prompt), "my-shell:%s> ", cwd);
    }else{
      snprintf(prompt, sizeof(prompt), "my-shell> ");
    }
    fputs(prompt, stdout);
    fflush(stdout);

    line = read_line(history, history_cnt);
    
    if(line == NULL){
      free(line);
      line = NULL;
      break;
    }

    int len = strlen(line);

    if(len > 0 && line[len-1] == '&'){
      line[len-1] = '\0';
      background = 1;
    }


    if(strcmp(line, "") != 0){
      history[history_cnt] = strdup(line);
      history_cnt++;
    }

    if(strcmp(line, "exit") == 0){
      free(line);
      line = NULL;
      break;
    }

    char* redir_out = strchr(line, '>');
    if(redir_out != NULL){
      *redir_out = '\0';
      execute_redir_out(line, redir_out+1);
      free(line);
      line = NULL;
      continue;
    }

    char *redir_in = strchr(line, '<');
    if(redir_in != NULL){
      *redir_in = '\0';
      execute_redir_in(line, redir_in+1);
      free(line);
      line = NULL;
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
      execute_pipe(cmds, n_cmds);
      free(line);
      line = NULL;
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
      free(line);
      line = NULL;
      continue;
    }

    if(strcmp(args[0], "help") == 0){
      builtin_help();
      free(line);
      line = NULL;
      continue;
    }

    if(strcmp(args[0], "history") == 0){
      builtin_history(history, history_cnt);
      free(line);
      line = NULL;
      continue;
    }

    if(strcmp(args[0], "cd") == 0){
      builtin_cd(args);
      free(line);
      line = NULL;
      continue;
    }


    execute_command(args, background);
    free(line);
    line = NULL;
  }
   free(line);
   return 0;
}
