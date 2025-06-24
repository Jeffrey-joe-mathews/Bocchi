#include "db.h"

void convert_from_binary(Row* row, void* source) {
    memcpy(&(row->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(row->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(row->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

ExecutionStatus execute_select(Table* table) {
    for (uint32_t i = 0; i < table->num_rows; i++) {
        Row row;
        convert_from_binary(&row, row_address(table, i));
        printf("(%d, %s, %s)\n", row.id, row.username, row.email);
    }
    return EXECUTE_SUCCESS;
}
