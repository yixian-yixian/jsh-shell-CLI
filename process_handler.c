#include "process_handler.h"

/* execute_cmd 
 * purpose: execute a child process 
 * notes: this function is called by launch_child_process
 *        when child process fails to launch:
 *              jsh error: Command not found: <command>
 */
void execute_cmd(char *cmd[]) {
    if (execvp(cmd[0], cmd) < 0) {
        fprintf(stderr, "jsh error: Command not found: %s\n", cmd[0]);
        exit(CHILD_PROCESS);
    } 
}

/* create_pipes 
 * purpose: create pipes for read and write ends 
 * notes: this function is called by launch_child_process
 *        when pipe creation fails:
 *          jsh error: unable to create pipe
 */
void create_pipes(int pipefds[], int num_pipes) {
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i*2) < 0) { 
            perror("jsh$ error: unable to create pipe");
            exit(CHILD_PROCESS);
        }
    }
}

/* close_all_pipes 
 * purpose: close all pipes between processes 
 * note: this function is called by redirect_execution
 */
void close_all_pipes(int pipefds[], int num_pipes) {
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]); 
    }
}

/* redirect_execution 
 * purpose: redirect stdin and stdout to appropriate pipes
 * notes: 
 * */
void redirect_execution(char *cmd[], int pipefds[], int i, int num_pipes) {
    // replace stdin with read end of previous pipe
    if (i != 0) {
        if (dup2(pipefds[(i-1)*2], 0) < 0) {
            perror("jsh$ error: unable to redirect stdin");
            exit(CHILD_PROCESS);
        }
    }
    // replace stdout with write end of current pipe
    if (i != num_pipes - 1) {
        if (dup2(pipefds[i*2 + 1], 1) < 0) {
            perror("jsh$ error: unable to redirect stdout");
            exit(CHILD_PROCESS);
        }
    }
    close_all_pipes(pipefds, num_pipes);
    execute_cmd(cmd);

}

/* launch_child_processes 
 * purpose: launch child processes and rediredct child processes 
 *          to appropriate pipes
 * notes: this function is called by process_input 
 *       when child process fails to launch:
 *         the function exit with status 127
 */
void launch_child_process(char *cmds[][MAX_CMD_LENGTH], int pipefds[], int num_pipes, pid_t pids[]) {
    int pid = 0;
    for (int i = 0; i < num_pipes; i++) {
        pid = fork();   // create child process
        if (pid == 0) { // child process created
            // if not the first command, close write end
            if (i != 0) {
                close(pipefds[(i-1)*2 + 1]);
            }
            // if not the last command, close read end
            if (i != num_pipes - 1) {
                close(pipefds[i*2]);
            }
            redirect_execution(cmds[i], pipefds, i, num_pipes);
        } else if (pid < 0) {
            fprintf(stderr, "Failed to create fork\n");
            exit(CHILD_PROCESS);
        } else {
            pids[i] = pid; // save pid of the child process
        }
    }
    // Close all remaining ends of the pipes in the parent process
    close_all_pipes(pipefds, num_pipes);
}



/* process_input
 * purpose: process user input and launch processes accordingly
 * notes: this function is called by main
*/
void process_input(char* user_input) {
    char stack_user_input[strlen(user_input)];
    strcpy(stack_user_input, user_input);
    free(user_input);

    int num_pipes = 0;
    // parse user input into pipes
    char *cmds[MAX_NUMBER_OF_PIPES][MAX_CMD_LENGTH];
    char *token = strtok(stack_user_input, "|"); 
    while(token) {
        int num_tokens = 0;
        char *saveptr2 = NULL;
        char *subtoken = strtok_r(token, " ", &saveptr2);
        while (subtoken) {
            cmds[num_pipes][num_tokens++] = subtoken;
            subtoken = strtok_r(NULL, " ", &saveptr2);
        } 
        cmds[num_pipes][num_tokens] = NULL;
        num_pipes++;
        token = strtok(NULL, "|");
    } 
    // create pipes
    int pipefds[2 * num_pipes];
    create_pipes(pipefds, num_pipes);


    // launch child processes 
    pid_t pids[MAX_NUMBER_OF_PIPES];
    launch_child_process(cmds, pipefds, num_pipes, pids);

    // // close all pipes
    close_all_pipes(pipefds, num_pipes);

    // wait for child process to finish 
    int child_status = 0;
    for (int i = 0; i < num_pipes; i++) {
        waitpid(pids[i], &child_status, 0);
    }
    
    if (WIFEXITED(child_status)) {
        fprintf(stdout, "jsh status: %d\n", WEXITSTATUS(child_status));
    } 
}



