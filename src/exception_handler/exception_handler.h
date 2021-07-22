#ifndef MOUSEDB_SRC_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_
#define MOUSEDB_SRC_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_

#include <assert.h>
#include <stdint.h>

#include <stdexcept>
#include <string>

// 定义状态码
enum StatusCode {
  SUCCESS,

  // DataFileHandler
  ERROR_FILE_NOT_EXISTS,
  ERROR_NOT_APPENDABLE,
  ERROR_NOT_READABLE,
  ERROR_WRONG_OPEN_MODE,
  ERROR_CAN_NOT_CACHE,
  ERROR_DEMANDING_PAGE_TO_CACHE_EXCEEDS_THE_FILE_SIZE,
  ERROR_DEMANDING_LINES_TO_READ_EXCEEDS_THE_FILE_SIZE,
  ERROR_CACHE_MAP_AND_LRU_INFO_DO_NOT_MATCH,

  // TableMetaHandler
  ERROR_TABLE_INFO_NOT_FOUND,
  ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META,

  // TablesHandler

  // DatabaseMetaHandler
  ERROR_DATABASE_HAS_ALREADY_EXISTS,
  ERROR_DATABASE_NOT_EXISTS,
  ERROR_TABLE_NOT_EXISTS,
  ERROR_TABLE_HAS_ALREADY_EXISTS,

  // DatabaseHandler

  // TableRow

  // CorrectnessChecker

  // UserAction
  ERROR_TABLE_NOT_OPENED,
  ERROR_OFFSET_EXCEEDS_THE_MAXIMUM,

  // Other
  ERROR_MEMORY_ALLOCATION_FAIL,

}; // enum status

// 定义返回数据与状态码的pair为respond
// respond的first为返回数据，second为状态码
template <typename T> using respond = std::pair<T, StatusCode>;

#endif // MOUSEDB_SRC_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_