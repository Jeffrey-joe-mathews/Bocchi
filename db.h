#ifndef DB_H
#define DB_H

#include <stdint.h>

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

const uint32_t ID_SIZE = szof(Row, id);                // 4 bytes
const uint32_t USERNAME_SIZE = szof(Row, username);    // 32 bytes
const uint32_t EMAIL_SIZE = szof(Row, email);          // 255 bytes

//
// -------- FIELD OFFSETS --------
// These define the byte offsets of each field in a binary row representation.
//

const uint32_t ID_OFFSET = 0;                                  // ID starts at 0
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;         // Username starts after ID
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;// Email starts after username

//
// -------- ROW SIZE --------
// Total number of bytes used by one row in memory.
//

const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

//
// -------- PAGE AND TABLE CONFIGURATION --------
// Simulate virtual memory paging behavior (4096 bytes = 4KB page size)
// and maximum number of rows supported.
//

const uint32_t PAGE_SIZE = 4096;          // Size of one page in bytes
#define MAX_TABLE_PAGES 100               // Max number of pages in the table
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;          // Rows that fit in one page
const uint32_t MAX_TABLE_ROWS = ROWS_PER_PAGE * MAX_TABLE_PAGES;  // Total table capacity

#endif // DB_H
