#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum
{
    SHELL_COMMAND_SUCCESS,
    SHELL_COMMAND_UNRECOGNIZED
} ShellCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum
{
    INSERT_STATEMENT,
    SELECT_STATEMENT
} StatementType;

typedef struct
{
    StatementType type;
} Statement;

InputBuffer *new_input_buffer()
{
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt()
{
    printf("[bocchi] >> ");
}

void read_input(InputBuffer *input_buffer)
{
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    // ssize_t is signed and can give negative values if something goes wrong hernce used for error handling over here
    // bytes_read contains the size which is also the length coincidentally because each character in char datatype has a size of 1
    if (bytes_read <= 0)
    {
        perror("Error in reading the input!\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void close_buffer(InputBuffer *input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}

ShellCommandResult do_shell_command(InputBuffer *input_buffer)
{
    if (((strcmp(input_buffer->buffer, ".exit")) == 0) || ((strcmp(input_buffer->buffer, ".close"))==0))
    {
        close_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return SHELL_COMMAND_UNRECOGNIZED;
    }
}

PrepareResult prepare_statement(InputBuffer *Input_buffer, Statement *statement)
{
    if (strncmp(Input_buffer->buffer, "insert", 6) == 0)
    {
        statement->type = INSERT_STATEMENT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(Input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = SELECT_STATEMENT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_recognized_statement(Statement *Statement)
{
    switch (Statement->type)
    {
    case INSERT_STATEMENT:
        printf("Insert somehing\n");
        break;

    case SELECT_STATEMENT:
        printf("select statement here\n");
        break;
    }
}

// 26-Mar-2025
// a database works in the principle of an infinite loop until exited... hence making something similar
// Just trying to ru infinite loop and accept text for now
int main(int argc, char *argv[])
{
    InputBuffer *input_buffer = new_input_buffer();
    while (true)
    {
        print_prompt();
        read_input(input_buffer);
        // if(strcmp(input_buffer->buffer, ".exit")==0 || strcmp(input_buffer->buffer, ".close")==0) {
        //     close_buffer(input_buffer);
        //     exit(EXIT_SUCCESS);
        // }
        // else {
        //     printf("Illegal Command : %s.\n", input_buffer->buffer);
        // }
        if (input_buffer->buffer[0] == '.')
        {
            switch (do_shell_command(input_buffer))
            {
            case SHELL_COMMAND_SUCCESS:
                continue;

            case SHELL_COMMAND_UNRECOGNIZED:
                printf("Illegal command señor : `%s`\n", input_buffer->buffer);
                continue;
            }
        }
        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            break;

        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("unrecognized keyword in '%s'.\n", input_buffer->buffer);
            continue;
        }
        execute_recognized_statement(&statement);
        printf("Executed.\n");
    }
    return 0;
}
