#ifndef DB_H
#define DB_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//
// -------- CONSTANTS FOR COLUMN SIZES --------
// These define the maximum size for username and email fields.
//

#define USERNAME_SIZE_TEMP_COLUMN 32     // Max 32 characters for username
#define EMAIL_SIZE_TEMP_COLUMN 255       // Max 255 characters for email

//
// -------- STRUCTURE DEFINITIONS --------
// Represents a row (record) in the table with three fields: id, username, and email.
//

typedef struct {
    uint32_t id;                                 // Integer ID (4 bytes)
    char username[USERNAME_SIZE_TEMP_COLUMN];    // Fixed-size username field
    char email[EMAIL_SIZE_TEMP_COLUMN];          // Fixed-size email field
} Row;

//
// -------- MACRO TO CALCULATE STRUCT FIELD SIZE --------
// Calculates the size of any field in a struct without needing an instance.
//

#define szof(struct, element) sizeof(((struct*)0)->element)

//
// -------- FIELD SIZE CONSTANTS --------
// These hold the size of each field within the Row struct.
//

#define ID_SIZE szof(Row, id)                // 4 bytes
#define USERNAME_SIZE szof(Row, username)    // 32 bytes
#define EMAIL_SIZE szof(Row, email)          // 255 bytes

//
// -------- FIELD OFFSETS --------
// These define the byte offsets of each field in a binary row representation.
//

#define ID_OFFSET 0                                        // ID starts at 0
#define USERNAME_OFFSET (ID_OFFSET + ID_SIZE)              // Username starts after ID
#define EMAIL_OFFSET (USERNAME_OFFSET + USERNAME_SIZE)     // Email starts after username

//
// -------- ROW SIZE --------
// Total number of bytes used by one row in memory.
//

#define ROW_SIZE (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)

//
// -------- PAGE AND TABLE CONFIGURATION --------
// Simulate virtual memory paging behavior (4096 bytes = 4KB page size)
// and maximum number of rows supported.
//

#define PAGE_SIZE 4096                     // Size of one page in bytes
#define MAX_TABLE_PAGES 100                // Max number of pages in the table
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)               // Rows that fit in one page
#define MAX_TABLE_ROWS (ROWS_PER_PAGE * MAX_TABLE_PAGES)   // Total table capacity

//
// -------- TYPE DEFINITIONS FOR INPUT AND PARSING --------
// These structures and enums define the input buffer, statement types,
// and execution status.
//

typedef struct {
    char *buffer;          // User input buffer
    size_t buffer_length;  // Allocated buffer length
    ssize_t input_length;  // Actual input length
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
    StatementType type;    // Command type: insert or select
    Row row_to_insert;     // Data to insert
} Statement;

typedef struct {
    uint32_t num_rows;              // Number of rows currently stored
    void* pages[MAX_TABLE_PAGES];   // Array of memory pages
} Table;

//
// -------- SHARED FUNCTION PROTOTYPES --------
// Functions used across files.
//

void* row_address(Table* table, uint32_t row_num);

#endif // DB_H
