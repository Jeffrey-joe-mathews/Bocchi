#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    size_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt() {
    printf("[bocchi] >> ");
}

void read_input(InputBuffer* input_buffer) {
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if(bytes_read<=0) {
        perror("Error in reading the input!\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read-1;
    input_buffer->buffer[bytes_read-1] = 0;
}

// 26-Mar-2025
// a database works in the principle of an infinite loop until exited... hence making something similar
// Just trying to ru infinite loop and accept text for now
int main (int argc, char *argv[]) {
    InputBuffer* input_buffer = new_input_buffer();
    while (true) { 
        print_prompt();
        read_input(input_buffer);
        
        if(strcmp(input_buffer->buffer, ".exit")==0 || strcmp(input_buffer->buffer, ".close")==0) {
            exit(EXIT_SUCCESS);
        }
        else {
            printf("Illegal Command : %s.\n", input_buffer->buffer);
        }
    }
    return 0;
}
