#include "handler/data_file_handler.h"

// 判断文件是否存在
#include <unistd.h>

#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

bool DataFileHandler::file_exist(const string &path)
{
    return access(path.c_str(), 0) == 0;
}

DataFileHandler::~DataFileHandler()
{
    if (file_.is_open())
        file_.close();
}

// 给成员变量file赋值
// 只要不可读，在文件不存在时，会创建文件
// 可读和可写同时出现，就没有创建属性
// truncate和append不能同时出现，否则文件会打开失败
// write：可写，成假时只读
// append：可写时从最后写
// truncate：可写时在打开时清空文件
// read：可读
void DataFileHandler::open(const string &file_path, bool write, bool append, bool truncate, bool read)
{
    ios::open_mode open_mode_of_file = ios::binary;

    if (write)
    {
        open_mode_of_file |= ios::out;

        if (append)
            open_mode_of_file |= ios::app;

        if (read)
            open_mode_of_file |= ios::in;

        // 若文件不存在，跳过truncate参数
        if (truncate && file_exist(file_path))
            open_mode_of_file |= ios::trunc;

        file_.open(file_path.c_str(), (std::ios_base::openmode)open_mode_of_file);
    }
    else // read only mode
    {
        open_mode_of_file |= ios::in;
        file_.open(file_path.c_str(), (std::ios_base::openmode)open_mode_of_file);
    }

    if (!file_.is_open())
        throw "DataFileHandler::open()：file not exists!";

    path_ = file_path;
    readable_ = read;
    appendable_ = append;
    writable_ = write;
    truncate_ = truncate;
}

void DataFileHandler::append(const string &string_to_write)
{
    if (!writable_)
        throw "DataFileHandler::append()：not appendable";

    if (!appendable_)
        file_.seekp(0, ios::end);
    
    file_.write(string_to_write.c_str(), string_to_write.size());
}

// 以string形式返回文件下一行的内容
// 若已经是最后一行，则string为空
// 读取，同时get指针会向下移一行
string DataFileHandler::read_line()
{
    if (writable_ && !readable_)
        throw "DataFileHandler::read_line()：not readable";

    string next_line;
    getline(file_, next_line);

    return next_line;
}

string DataFileHandler::read_all()
{
    if (writable_ && !readable_)
        throw "DataFileHandler::read_line()：not readable";

    // 移动至末尾
    file_.seekg(0, ios::end);

    // 获取位置，位置即文件长度
    int buffer_length = file_.tellg();

    // 回到开头
    file_.seekg(0, ios::beg);

    char *char_file_content = new char[buffer_length];
    file_.read(char_file_content, buffer_length);

    string file_content = char_file_content;

    return file_content;
}

bool DataFileHandler::is_open()
{
    return file_.is_open();
}