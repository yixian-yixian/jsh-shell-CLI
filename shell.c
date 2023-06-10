#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_CMD_LENGTH 100
#define MAX_NUMBER_OF_PIPES 1024
#define CHILD_PROCESS 127


/* execute_cmd 
 * purpose: execute a child process 
 * notes: this function is called by launch_child_process
 */
void execute_cmd(char *cmd[]) {
    if (execvp(cmd[0], cmd) < 0) {
        perror(cmd[0]);
        exit(CHILD_PROCESS);
    }
}

/* create_pipes 
 * purpose: create pipes for read and write ends 
 * notes: 
 */
void create_pipes(int pipefds[], int num_pipes) {
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i*2) < 0) { // TODO: check expected error output
            exit(CHILD_PROCESS);
        }
    }
}

/* close_all_pipes 
 * purpose: close all pipes between processes 
 * note: 
 */
void close_all_pipes(int pipefds[], int num_pipes) {
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]); }
}

/* redirect_execution 
 * purpose: redirect execution of child process
 * notes: 
 * */
void redirect_execution(char *cmd[], int pipefds[], int i, int num_pipes) {
    // replace stdin with read end of previous pipe
    if (i != 0) {
        if (dup2(pipefds[(i-1)*2], 0) < 0) {
            perror("dup2"); // stdin
            exit(CHILD_PROCESS);
        }
    }

    // replace stdout with write end of current pipe
    if (i != num_pipes - 1) {
        if (dup2(pipefds[i*2 + 1], 1) < 0) {
            perror("dup2"); // stdout
            exit(CHILD_PROCESS);
        }
    }
    close_all_pipes(pipefds, num_pipes);
    execute_cmd(cmd);

}

/* launch_child_processes 
 * purpose: launch child processes that will execute commands
 * notes: exit status 127 if child process creation fails
 */
void launch_child_process(char *cmds[][MAX_CMD_LENGTH], int pipefds[], int num_pipes) {
    int pid = 0;
    for (int i = 0; i < num_pipes; i++) {
        // create child process
        pid = fork();
        if (pid == 0) { // child process created
            redirect_execution(cmds[i], pipefds, i, num_pipes);
        } else if (pid < 0) {
            exit(CHILD_PROCESS);
        }
    }
}


void process_input(char *user_input) {
    int num_pipes = 0;
    // parse user input into pipes
    char *cmds[MAX_NUMBER_OF_PIPES][MAX_CMD_LENGTH];
    char *token = strtok(user_input, "|");

    // TODO test edge cases with odd command input 
    while (token) {
        int num_tokens = 0;
        char *subtoken = strtok(token, " "); // TODO check if to replace with strtok_r
        while (subtoken) {
            cmds[num_pipes][num_tokens++] = subtoken;
            subtoken = strtok(NULL, " ");
        }
        cmds[num_pipes][num_tokens] = NULL;
        num_pipes++;
        token = strtok(NULL, "|");
    }
    // create pipes
    int pipefds[2 * num_pipes];
    create_pipes(pipefds, num_pipes);

    // launch child processes
    launch_child_process(cmds, pipefds, num_pipes);

    // close pipes
    close_all_pipes(pipefds, num_pipes);
    int status = 0;
    for (int i = 0; i < num_pipes + 1; i++) {
        wait(&status);
    }

    
}


/* driver function */
int main() {
    char user_input[MAX_CMD_LENGTH];
    fprintf(stdout, "jsh$ ");
    while (fgets(user_input, MAX_CMD_LENGTH, stdin) != NULL) {
        // remove newline from user input
        user_input[strcspn(user_input, "\n")] = 0;
        if (strcmp(user_input, "exit") == 0) 
            exit(0);
        else 
            process_input(user_input);
    
        fprintf(stdout, "jsh$ ");
    }
    return 0;
}