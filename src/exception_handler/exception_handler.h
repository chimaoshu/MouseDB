#ifndef MOUSEDB_SRC_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_
#define MOUSEDB_SRC_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_

#include <assert.h>
#include <stdint.h>

#include <stdexcept>
#include <string>

// 定义状态码类型
typedef uint8_t status_code;

// 定义返回数据与状态码的pair为respond
// respond的first为返回数据，second为状态码
template <typename T> using respond = std::pair<T, status_code>;

// 定义状态码
namespace status {
const status_code SUCCESS = 0;

// DataFileHandler
const status_code ERROR_FILE_NOT_EXISTS = 1;
const status_code ERROR_NOT_APPENDABLE = 2;
const status_code ERROR_NOT_READABLE = 3;
const status_code ERROR_WRONG_OPEN_MODE = 4;
const status_code ERROR_CAN_NOT_CACHE = 5;
const status_code ERROR_DEMANDING_PAGE_TO_CACHE_EXCEEDS_THE_FILE_SIZE = 6;
const status_code ERROR_DEMANDING_LINES_TO_READ_EXCEEDS_THE_FILE_SIZE = 7;
const status_code ERROR_CACHE_MAP_AND_LRU_INFO_DO_NOT_MATCH = 8;

// TableMetaHandler
const status_code ERROR_TABLE_INFO_NOT_FOUND = 11;
const status_code ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META = 12;

// TablesHandler

// DatabaseMetaHandler
const status_code ERROR_DATABASE_HAS_ALREADY_EXISTS = 31;
const status_code ERROR_DATABASE_NOT_EXISTS = 32;
const status_code ERROR_TABLE_NOT_EXISTS = 33;
const status_code ERROR_TABLE_HAS_ALREADY_EXISTS = 34;

// DatabaseHandler

// TableRow

// CorrectnessChecker

// UserAction
const status_code ERROR_TABLE_NOT_OPENED = 72;
const status_code ERROR_OFFSET_EXCEEDS_THE_MAXIMUM = 73;

// Other
const status_code ERROR_MEMORY_ALLOCATION_FAIL = 81;

} // namespace status

#endif // MOUSEDB_SRC_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_