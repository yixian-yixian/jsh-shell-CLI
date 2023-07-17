#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CMD_LENGTH 1000
#define MAX_NUMBER_OF_PIPES 40
#define CHILD_PROCESS 127

void process_input(char* user_input);
void close_all_pipes(int pipefds[], int num_pipes);
void create_pipes(int pipefds[], int num_pipes);
void execute_cmd(char *cmd[]);
void redirect_execution(char* cmd[], int pipefds[],
    int i, int num_pipes);
void launch_child_process(char *cmds[][MAX_CMD_LENGTH], 
    int pipefds[], int num_pipes, pid_t pids[]);