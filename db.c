#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "db.h"

#ifdef _WIN32
// Custom implementation of getline for Windows
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (lineptr == NULL || n == NULL || stream == NULL) return -1;

    char *buf = *lineptr;
    size_t size = *n;
    int ch;
    size_t len = 0;

    if (buf == NULL || size == 0) {
        size = 128;
        buf = malloc(size);
        if (buf == NULL) return -1;
        *lineptr = buf;
        *n = size;
    }

    while ((ch = fgetc(stream)) != EOF) {
        if (len + 1 >= size) {
            size *= 2;
            char *new_buf = realloc(buf, size);
            if (!new_buf) return -1;
            buf = new_buf;
            *lineptr = buf;
            *n = size;
        }
        buf[len++] = ch;
        if (ch == '\n') break;
    }

    if (len == 0) return -1;

    buf[len] = '\0';
    return len;
}
#endif

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    SHELL_COMMAND_SUCCESS,
    SHELL_COMMAND_UNRECOGNIZED
} ShellCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    INSERT_STATEMENT,
    SELECT_STATEMENT
} StatementType;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_FAILURE,
    EXECUTE_TABLE_FULL
} ExecutionStatus;

typedef struct {
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
    for (uint32_t i = 0; i < MAX_TABLE_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

void freeTable(Table* table) {
    for (uint32_t i = 0; i < MAX_TABLE_PAGES; i++) {
        free(table->pages[i]);
    }
    free(table);
}

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
    if (bytes_read <= 0) {
        perror("Error in reading the input!\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0; // Remove newline
}

void close_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

ShellCommandResult do_shell_command(InputBuffer* input_buffer, Table* table) {
    if ((strcmp(input_buffer->buffer, ".exit") == 0) || (strcmp(input_buffer->buffer, ".close") == 0)) {
        close_buffer(input_buffer);
        freeTable(table);
        exit(EXIT_SUCCESS);
    } else {
        return SHELL_COMMAND_UNRECOGNIZED;
    }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = INSERT_STATEMENT;
        int args_assigned = sscanf(
            input_buffer->buffer,
            "insert %d %31s %254s",
            &(statement->row_to_insert.id),
            statement->row_to_insert.username,
            statement->row_to_insert.email
        );
        if (args_assigned < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }

    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = SELECT_STATEMENT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void convert_to_binary(Row* row, void* destination) {
    memcpy(destination + ID_OFFSET, &(row->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
}

void convert_from_binary(Row* row, void* source) {
    memcpy(&(row->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(row->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(row->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* row_address(Table* table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table->pages[page_num];

    if (page == NULL) {
        page = malloc(PAGE_SIZE);
        table->pages[page_num] = page;
    }

    uint32_t row_offset = (row_num % ROWS_PER_PAGE) * ROW_SIZE;
    return page + row_offset;
}

ExecutionStatus execute_insert(Statement* statement, Table* table) {
    if (table->num_rows >= MAX_TABLE_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);
    convert_to_binary(row_to_insert, row_address(table, table->num_rows));
    table->num_rows++;

    return EXECUTE_SUCCESS;
}

ExecutionStatus execute_recognized_statement(Statement* statement, Table* table) {
    switch (statement->type) {
        case INSERT_STATEMENT:
            return execute_insert(statement, table);
        case SELECT_STATEMENT:
            for (uint32_t i = 0; i < table->num_rows; i++) {
                Row row;
                convert_from_binary(&row, row_address(table, i));
                printf("(%d, %s, %s)\n", row.id, row.username, row.email);
            }
            return EXECUTE_SUCCESS;
    }
    return EXECUTE_FAILURE;
}

int main(int argc, char* argv[]) {
    Table* table = newTable();
    InputBuffer* input_buffer = new_input_buffer();

    while (true) {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.') {
            switch (do_shell_command(input_buffer, table)) {
                case SHELL_COMMAND_SUCCESS:
                    continue;
                case SHELL_COMMAND_UNRECOGNIZED:
                    printf("Illegal command señor: '%s'\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error near: '%s'\n", input_buffer->buffer);
                printf("Usage: insert <int> <username (max 31)> <email (max 254)>\n");
                continue;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized statement: '%s'\n", input_buffer->buffer);
                continue;
        }

        switch (execute_recognized_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
            case EXECUTE_FAILURE:
                printf("Execution failed.\n");
                break;
        }
    }

    return 0;
}
