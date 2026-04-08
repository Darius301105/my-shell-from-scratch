#ifndef UTILS_H
#define UTILS_H

void handle_sigchld(int sig);
void handle_sigint(int sig);
void builtin_cd(char ** args);
void builtin_history(char **history, int count);
void builtin_help();
void execute_command(char **args, int background);
void execute_pipe(char **cmds, int n_cmds);
void execute_redir_out(char *cmd_str, char *file_str);
void execute_redir_in(char *cmd_str, char *file_str);

#endif
