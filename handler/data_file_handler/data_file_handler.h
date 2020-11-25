#ifndef MOUSEDB_HANDLER_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_
#define MOUSEDB_HANDLER_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_

// 用来判断文件是否存在
#include <unistd.h>

#include <string>
#include <fstream>
#include <iostream>

#include "handler/exception_handler/exception_handler.h"
#include "lib/Json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

// 文件处理类
// 使用默认构造函数，因此需要在实例化后自行调用open打开文件，再进行后续操作。
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

    // 被打开的文件
    fstream file_;

public:
    DataFileHandler() = default;
    ~DataFileHandler();

    // 路径下的文件是否存在
    bool file_exist(const string &path);

    // 返回文件路径
    string get_path();

    // 给成员变量file_赋值
    // 若不可读，那么在文件不存在时，就会创建文件
    // 若可读和可写同时出现，那么没有创建属性
    // truncate和append不能同时出现，否则文件会打开失败
    // write：可写，成假时只读
    // append：可写时从最后写
    // truncate：可写时在打开时清空文件
    // read：可读
    status_code open(const string &file_path, bool write = 1, bool append = 0, bool truncate = 0, bool read = 0);

    bool is_open();

    // 关闭file_，但不析构，可以重新open。
    void close();

    // 在文件末尾追加字符串
    status_code append(const string &string_to_write);

    // 在文件末尾追加二进制数据
    status_code append(void *buffer, int buffer_size);

    // 从initial_adress开始，以每行line_size的大小，读number_of_line行
    // 返回指向内存的指针与读取到的可用行数
    pair<void *, uint64_t> read_lines_into_buffer(
        const int &initial_adress,
        const int &line_size,
        const int &number_of_line);

    // 读取一行字符串，返回字符串和状态码
    // 在异常或者已读取到底的情况下，字符串为空
    respond<string> read_line();

    // 读取整个文件内容，返回字符串和状态码
    // 在异常或者已读取到底的情况下，字符串为空
    respond<string> read_all();
};
#endif // MOUSEDB_HANDLER_DATA_FILE_HANDLER_H_