#include <stdio.h>
#include <stdlib.h>
#include "process_handler.h"

/* driver function */
int main() {
    char* user_input = calloc(MAX_CMD_LENGTH, sizeof(char));
    size_t buf_size = MAX_CMD_LENGTH;
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
            user_input[input_length++] = '\0';
            // exit program if user enters "exit"
            if (strcmp(user_input, "exit") == 0) {
                free(user_input);
                exit(0);
            }
            else {
                process_input(user_input);
            }
            user_input = calloc(MAX_CMD_LENGTH, sizeof(char));
            input_length = 0; // reset input length
            fprintf(stdout, "jsh$ ");
        }
        else // add char to user_input
            user_input[input_length++] = c;
    }

    free(user_input);
    return 0;
}
