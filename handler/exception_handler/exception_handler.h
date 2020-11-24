#ifndef MOUSEDB_HANDLER_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_
#define MOUSEDB_HANDLER_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_

#include <stdint.h>
#include <assert.h>

#include <stdexcept>
#include <string>

// 定义状态码类型
typedef int8_t status_code;

// 定义返回数据与状态码的pair为respond
// respond的first为返回数据，second为状态码
template <typename T>
using respond = std::pair<T, status_code>;

// 有些error不能被忽略，这种error的出现意味着bug的存在
void raise_exception(status_code code);

// 定义状态码
namespace error_code
{
    const status_code SUCCESS = 0;

    // 文件操作类
    const status_code ERROR_FILE_NOT_EXISTS = 1;
    const status_code ERROR_NOT_APPENDABLE = 2;
    const status_code ERROR_NOT_READABLE = 3;
    const status_code ERROR_WRONG_OPEN_MODE = 4;

    // 数据库元数据JSON操作类
    const status_code ERROR_DATABASE_HAS_ALREADY_EXISTS = 5;
    const status_code ERROR_DATABASE_NOT_EXISTS = 6;
    const status_code ERROR_TABLE_NOT_EXISTS = 7;
    const status_code ERROR_TABLE_HAS_ALREADY_EXISTS = 8;

    // 表元数据JSON操作类
    const status_code ERROR_TABLE_INFO_NOT_FOUND = 9;
    const status_code ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META = 10;

    // 内存分配错误
    const status_code ERROR_MEMORY_ALLOCATION_FAIL = 11;

    // 数据库元数据的合法性

    // 表元数据合法性

    // 用户操作
    const status_code ERROR_TABLE_NOT_OPENED = 12;

    // 查询
    const status_code ERROR_OFFSET_EXCEEDS_THE_MAXIMUM = 13;

} // namespace error_code

#endif // MOUSEDB_HANDLER_EXCEPTION_HANDLER_EXCEPTION_HANDLER_H_