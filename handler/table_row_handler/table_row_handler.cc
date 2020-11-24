#include "table_row_handler.h"

TableRowHandler::TableRowHandler(const string &file_path, TableMetaHandler *&table_meta_handler)
    : table_name_(table_meta_handler->get_table_name()),
      line_size_(table_meta_handler->get_line_size()),
      off_set_of_each_column_(table_meta_handler->get_off_set_of_each_column()),
      type_of_each_column_(table_meta_handler->get_type_of_each_column()),
      table_meta_(table_meta_handler->get_table_meta())

{
    file_.open(file_path, 1, 1, 0, 1);
}

template <class T>
T *TableRowHandler::read(uint64_t &off_set, uint64_t &line_number, list<int> wanted_columns)
{
    pair<void *, uint64_t> buffer_info = file_.read_lines_into_buffer(off_set * line_size_, line_size_, line_number);

    T *rows = new T;

    void *buffer_pointer = buffer_info.first;
    uint64_t available_lines = buffer_info.second;

    // 解析buffer
    for (int i = 0; i < available_lines; i++)
    {
        // 存储一行数据
        json row;

        // 读取想要的行
        for (auto it = wanted_columns.begin(); it != wanted_columns.end(); it++)
        {
            // 变量的初始读取地址
            int off_set_address = i * line_size_ + off_set_of_each_column_[*it];
            void *initial_address = buffer_pointer + off_set_address;

            // 传入变量ID，映射为模板元变量，转化指针类型读取后push_back到row中
            // 具体实现参考buffer_reader.h
            read_buffer<10>(type_of_each_column_[*it], initial_address, row);
        }
        rows->push_back(row);
    }

    if (buffer_pointer)
    {
        delete[](char *) buffer_pointer;
    }

    return rows;
}

template <class T>
status_code TableRowHandler::write(const T &rows_information)
{
    // 行数目
    int rows_number = rows_information.size();

    // 需要写入的数据所占的内存
    int buffer_size = line_size_ * rows_number;

    void *buffer = new char[buffer_size];

    if (!buffer)
    {
        return error_code::ERROR_MEMORY_ALLOCATION_FAIL;
    }

    // 遍历每一行
    int row_order = 0;
    for (auto it_of_rows = rows_information.begin(); it_of_rows != rows_information.end(); it_of_rows++, row_order++)
    {
        const json &row = *it_of_rows;

        // 列序号，与数据类型一一对应
        // 从1开始
        int column_order = 1;

        // 遍历每一列
        for (auto it = row.begin(); it != row.end(); it++, column_order++)
        {
            // 计算从buffer写入该变量的初始地址
            int off_set_address = line_size_ * row_order + off_set_of_each_column_[column_order];
            void *initial_address = buffer + off_set_address;

            // 写入buffer
            write_buffer<10>(type_of_each_column_[column_order], initial_address, it);
        }
    }

    status_code code = file_.append(buffer, buffer_size);

    if (buffer)
    {
        delete[](char *) buffer;
    }

    return code;
}

// 特化

template json *TableRowHandler::read(uint64_t &off_set, uint64_t &line_number, list<int> wanted_columns);
template vector<json> *TableRowHandler::read(uint64_t &off_set, uint64_t &line_number, list<int> wanted_columns);

template status_code TableRowHandler::write(const vector<json> &rows_information);
template status_code TableRowHandler::write(const json &rows_information);