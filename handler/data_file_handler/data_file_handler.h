#ifndef MOUSEDB_HANDLER_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_
#define MOUSEDB_HANDLER_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_

// 用来判断文件是否存在
#include <unistd.h>

#include <string>
#include <fstream>

#include "handler/exception_handler/exception_handler.h"

using namespace std;

// 文件处理类
// 使用默认构造函数，因此需要在实例化后自行调用open打开文件，再进行后续操作。
// 类方法若返回异常，请检查open_mode
class DataFileHandler
{
private:
    // 路径
    string path_;

    // 打开模式
    bool writable_;
    bool readable_;
    bool appendable_;
    bool truncate_;

    // 打开的文件
    fstream file_;

    // 路径下的文件是否存在
    bool file_exist(const string &path);

public:
    DataFileHandler() = default;
    ~DataFileHandler();

    // 返回文件路径，可读不可写
    const string &get_path();

    // 给成员变量file赋值
    // 只要不可读，在文件不存在时，会创建文件
    // 可读和可写同时出现，就没有创建属性
    // truncate和append不能同时出现，否则文件会打开失败
    // write：可写，成假时只读
    // append：可写时从最后写
    // truncate：可写时在打开时清空文件
    // read：可读
    status_code open(const string &file_path, bool write = 1, bool append = 0, bool truncate = 0, bool read = 0);

    // 文件是否已经打开
    bool is_open();

    // 在文件末尾追加字符串
    // 以string形式返回文件下一行的内容
    // 若已经是最后一行，则string为空
    // 同时get指针会向下移一行
    status_code append(const string &string_to_write);

    // TODO：在文件末尾追加二进制数据
    // void append();

    // 读取一行字符串，返回字符串和状态码
    // 在异常或者已读取到底的情况下，字符串为空
    respond<string> read_line();

    // TODO：读取数据库一行内容
    // read_line()

    // 读取整个文件内容，返回字符串和状态码
    // 在异常或者已读取到底的情况下，字符串为空
    respond<string> read_all();
};
#endif // MOUSEDB_HANDLER_DATA_FILE_HANDLER_H_