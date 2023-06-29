#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_CMD_LENGTH 1000
#define MAX_NUMBER_OF_PIPES 20
#define CHILD_PROCESS 127


/* execute_cmd
 * purpose: execute a child process
 * notes: this function is called by launch_child_process
 *        when child process fails to launch:
 *              jsh error: Command not found: <command>
 */
void execute_cmd(char* cmd[]) {
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
        if (pipe(pipefds + i * 2) < 0) {
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
        if (close(pipefds[i]) < 0) {
            perror("jsh$ error: failed to close pipe");
            exit(1);
        }
    }
}

/* redirect_execution
 * purpose: redirect stdin and stdout to appropriate pipes
 * notes:
 * */
void redirect_execution(char* cmd[], int pipefds[],
    int i, int num_pipes) {
    // replace stdin with read end of previous pipe
    if (i != 0) {
        if (dup2(pipefds[(i - 1) * 2], 0) < 0) {
            perror("jsh$ error: unable to redirect stdin");
            exit(CHILD_PROCESS);
        }
    }
    // replace stdout with write end of current pipe
    if (i != num_pipes - 1) {
        if (dup2(pipefds[i * 2 + 1], 1) < 0) {
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
void launch_child_process(char** cmds[], int pipefds[], pid_t pids[], int num_pipes) {
    pid_t pid = 0;
    for (int i = 0; i < num_pipes; i++) {
        pid = fork();   // create child process
        if (pid == 0) { // child process created
            redirect_execution(cmds[i], pipefds, i, num_pipes);
        }
        else if (pid < 0) {
            exit(CHILD_PROCESS);
        }
        pids[i] = pid;
    }
}

/* process_input
 * purpose: process user input and launch processes accordingly
 * notes: this function is called by main
*/
void process_input(char* user_input) {
    int num_pipes = 0;

    // parse user input into pipes
    char** cmds[MAX_NUMBER_OF_PIPES];
    for (int i = 0; i < MAX_NUMBER_OF_PIPES; i++) {
        cmds[i] = malloc(sizeof(char*) * CMD_LENGTH);
        if (!cmds[i]) {
            perror("malloc");
            exit(1);
        }
    }
    char* token = strtok(user_input, "|");

    while (token) {
        int num_tokens = 0;
        char* saveptr2 = NULL;
        char* subtoken = strtok_r(token, " ", &saveptr2);
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
    launch_child_process(cmds, pipefds, pids, num_pipes);

    // close pipes
    close_all_pipes(pipefds, num_pipes);

    // wait for last child process to finish
    int child_status = 0;
    for (int i = 0; i < num_pipes; i++) {
        if (waitpid(pids[i], &child_status, 0) == -1) {
            perror("waitpid");
            exit(1);
        }
    }
    if (WIFEXITED(child_status))
        fprintf(stdout, "jsh status: %d\n", WEXITSTATUS(child_status));

    // free allocated parsed cmd memory
    for (int i = 0; i < num_pipes; i++) {
        free(cmds[i]);
    }
}

/* driver function */
int main() {
    char* user_input = calloc(CMD_LENGTH, sizeof(char));
    size_t buf_size = CMD_LENGTH;
    size_t input_length = 0;
    fprintf(stdout, "jsh$ ");

    // read user input char by char
    int c;
    while (1) {
        c = fgetc(stdin);
        if (c == EOF && ferror(stdin)) {
            perror("fgetc");
            free(user_input);
            exit(1);
        }
        // expand buffer size if full
        if (input_length == buf_size - 1) {
            buf_size *= 2;
            user_input = realloc(user_input, buf_size * sizeof(char));
            if (!user_input) {
                perror("realloc");
                exit(1);
            }
        }
        if (c == '\n') {
            user_input[input_length] = '\0';
            // exit program if user enters "exit"
            if (strcmp(user_input, "exit") == 0) {
                free(user_input);
                exit(0);
            }
            else {
                process_input(user_input);
            }
            input_length = 0; // reset input length
            fprintf(stdout, "jsh$ ");
        }
        else // add char to user_input
            user_input[input_length++] = c;
    }

    free(user_input);
    return 0;
}
