#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define USERNAME_SIZE_TEMP_COLUMN 32
#define EMAIL_SIZE_TEMP_COLUMN 255

typedef struct
{
    uint32_t id;
    char username[USERNAME_SIZE_TEMP_COLUMN];
    char email[EMAIL_SIZE_TEMP_COLUMN];
} Row;

#define szof(struct, element) sizeof(((struct*)0)->element) // calculates the size of any field of the structure using a temporary null pointer

const uint32_t ID_SIZE = szof(Row, id);
const uint32_t USERNAME_SIZE = szof(Row, username);
const uint32_t EMAIL_SIZE = szof(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET+ID_SIZE;
const uint32_t EMAIL_OFFSET= USERNAME_OFFSET+USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096; // standard number of bytes 8^4 or 2^12 : used in most modern system architectures(4KB)
#define MAX_TABLE_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t MAX_TABLE_ROWS = ROWS_PER_PAGE * MAX_TABLE_PAGES;

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
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum
{
    INSERT_STATEMENT,
    SELECT_STATEMENT
} StatementType;

typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_FAILURE,
    EXECUTE_TABLE_FULL
} ExecutionStatus;

typedef struct
{
    StatementType type;
    Row row_to_insert;
} Statement;

typedef struct {
    uint32_t num_rows;
    void* pages[MAX_TABLE_PAGES];
} Table;

Table* newTable() {
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0;i<MAX_TABLE_PAGES;i++) {
        table->pages[i] = NULL;
    }
    return table;
}

void freeTable(Table* table) {
    for (uint32_t i = 0; i<MAX_TABLE_PAGES;i++) {
        free(table->pages[i]);
    }
    free(table);
}

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

ShellCommandResult do_shell_command(InputBuffer *input_buffer, Table *disposableTable)
{
    if (((strcmp(input_buffer->buffer, ".exit")) == 0) || ((strcmp(input_buffer->buffer, ".close"))==0))
    {
        close_buffer(input_buffer);
        freeTable(disposableTable);
        exit(EXIT_SUCCESS); // or you could do return SHELL_COMMAND_SUCCESS
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
        int args_count = sscanf(
            Input_buffer->buffer, 
            "insert %d %31s %254s", 
            &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email
        );
        if(args_count<3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strncmp(Input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = SELECT_STATEMENT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void convert_to_binary(Row* row, void* destination) {
    memcpy(destination+ID_OFFSET, &(row->id), ROW_SIZE);
    memcpy(destination+USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
    memcpy(destination+EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
}

void convert_from_binary(Row* row, void* source) {
    memcpy(&(row->id), source+ID_OFFSET, ID_SIZE);
    memcpy(&(row->username), source+USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(row->email), source+EMAIL_OFFSET, EMAIL_SIZE);
}

void* row_address (Table* table, uint32_t row_num) {
    uint32_t page_number = row_num/ROWS_PER_PAGE;
    void* page = table->pages[page_number];
    if(page == NULL)
        page = table->pages[page_number] = malloc(PAGE_SIZE);
    uint32_t offset = (row_num % ROWS_PER_PAGE) * ROW_SIZE; 
    return page + offset;
}

ExecutionStatus execute_insert(Statement *statement, Table *table) {
    if (table->num_rows >= MAX_TABLE_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);

    convert_to_binary(row_to_insert, row_address(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;

}

ExecutionStatus execute_recognized_statement(Statement *statement, Table *table)
{
    switch (statement->type)
    {
    case INSERT_STATEMENT:
        return execute_insert(statement, table);

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
    Table *table = newTable();
    InputBuffer *input_buffer = new_input_buffer();
    while (true)
    {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.')
        {
            switch (do_shell_command(input_buffer, table))
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
        case PREPARE_SYNTAX_ERROR:
            printf("Could not Tokenize the statement '%s'.\n",input_buffer->buffer);
            if(statement.type == INSERT_STATEMENT) {
                printf("The syntax for insertion is:\n'insert <integer_id> <32bitString_username> <254bitString_email>' obv without the ''\n");
            }
            continue;
        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("unrecognized keyword in '%s'.\n", input_buffer->buffer);
            continue;
        }

        switch(execute_recognized_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                printf("EXECUTED\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("ERROR!!! TABLE FULL\n");
                break;
        }
    }
    return 0;
}
