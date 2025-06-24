#include "db.h"

void convert_to_binary(Row* row, void* destination) {
    memcpy(destination + ID_OFFSET, &(row->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
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
