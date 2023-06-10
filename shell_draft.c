#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#define DEFAULT 100
#define BUILTIN "exit"

// child functions 
void launch_child_process(); 
void exit_program(char **memory_addr, int exit_status) {
    if (*memory_addr != NULL) {
        free(*memory_addr);
    }
    free(memory_addr);
    exit(exit_status);
}

int main(int argc, char *argv[]) {
    // print a prompt to terminal
    char *user_input = calloc(sizeof(char), DEFAULT);
    char *token = NULL, *saveptr = NULL;
    assert(user_input != NULL);
    fprintf(stdout, "jsh$ ");

    // collect user response 
    while (fgets(user_input, DEFAULT, stdin) != NULL) {
        // fprintf(stderr, "current user input: [%s] \n", user_input);
        user_input[strcspn(user_input, "\n")] = 0;
        if (!strcmp(user_input, BUILTIN)) { // built in command 
            exit_program(&user_input, 0);
        } else { // user defined action
            // parse user command ++ format into request 
            char *exec_fields[DEFAULT];
            int cnt = 0;
            token = strtok_r(user_input, " ", &saveptr); 
            while (token != NULL) {
                exec_fields[cnt++] = strdup(token); 
                token = strtok_r(NULL, " ", &saveptr);
            }
            exec_fields[cnt] = NULL;
            // launching child process
            launch_child_process(exec_fields);
            // free the instruction array
            for (int i = 0; i < cnt; i++) {
                free(exec_fields[i]); // free the instruction array
            }
            
        }
        // clear user input
        memset(user_input, 0, DEFAULT); // Clear the user input
        fprintf(stdout, "jsh$ ");
    }
    exit_program(&user_input, 0);
}


void launch_child_process(char *use_command[]) {
    // forking a child process 
    int pid = fork(), child_status;
    if (pid < 0) {
        perror("fork failed");
        exit(127);
    } else if (pid == 0) { 
        // child process execution 
        fprintf(stderr, "launched child process: (pid:%d)\n", (int) getpid());
        if (execvp(use_command[0], use_command) < 0) {
            // TODO fix proper error handling   
            fprintf(stderr, "jsh: %s: %s\n", use_command[0], strerror(errno));
            exit(127);
        }
    } else {  
        // wait for child process to finish 
        waitpid(pid, &child_status, 0);
        do {
            if (WIFEXITED(child_status)) {
                fprintf(stdout, "jsh status: %d\n", WEXITSTATUS(child_status));
            } else if (WIFSIGNALED(child_status)) {
                fprintf(stdout, "child killed (signal %d)\n", WTERMSIG(child_status));
            } else if (WIFSTOPPED(child_status)) {
                fprintf(stdout, "child stopped (signal %d)\n", WSTOPSIG(child_status));
        }} while (!WIFEXITED(child_status) && !WIFSIGNALED(child_status));
        
    }

}

