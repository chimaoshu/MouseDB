#include "src/table_row_handler/table_row_handler.h"

using namespace std;
using json = nlohmann::json;

TableRowHandler::TableRowHandler(const string &file_path,
                                 TableMetaHandler *&table_meta_handler,
                                 bool truncate = false,
                                 uint8_t cache_pages = 0)
    : file_(cache_pages), // 默认缓存n页内容
      table_name_(table_meta_handler->get_table_name()),
      line_size_(table_meta_handler->get_line_size()),
      off_set_of_each_column_(table_meta_handler->get_off_set_of_each_column()),
      type_of_each_column_(table_meta_handler->get_type_of_each_column()),
      primary_key_number_(table_meta_handler->get_primary_key_number())

{
    if (truncate)
        // 若文件不存在，则创建；若文件存在，则清空。
        // 情景：1、写入热数据 2、热数据dump为新的冷数据
        file_.open(file_path, 1, 0, 1, 0);
    else
        file_.open(file_path, 1, 0, 0, 0);

    file_.close();

    // 以读、写、追加方式打开
    file_.open(file_path, 1, 1, 0, 1);

    // g游标一开始会处于文件头部，方便按顺序读
    // p游标会始终处于文件尾部，方便追加
    file_.move_g_cursor();
}

inline row_order TableRowHandler::get_number_of_rows_in_file()
{
    uint64_t file_size = file_.get_file_size();
    assert(file_size % line_size_ == 0);
    return file_size / line_size_;
}

template <class T>
T *TableRowHandler::read_and_serialize(uint32_t &off_set, uint32_t &line_number, list<int> wanted_columns)
{
    pair<void *, uint32_t> buffer_info = file_.read_lines_into_buffer(off_set * line_size_,
                                                                      line_size_,
                                                                      line_number);

    T *rows = new T;

    void *buffer_pointer = buffer_info.first;
    uint32_t available_lines = buffer_info.second;

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
status_code TableRowHandler::deserialize_and_write(const T &rows_information)
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

        // 遍历每一个key
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

// 读取一行的数据，返回pair
// pair第一个指针指向读取那行的内存
// pair第二个指针指向由primary_key构成的vector的内存
// 使用完毕需要释放内存
inline pair<void *, rbtree_key *> TableRowHandler::read_next_row_buffer_and_index()
{
    pair<void *, uint32_t> buffer_info = file_.read_lines_into_buffer(0, line_size_, 1, true);

    // 可用行数为0，表示读取到文件末尾了，结束了
    if (buffer_info.second == 0)
        return pair<void *, rbtree_key *>(NULL, NULL);

    // 内存的起始地址
    void *buffer_pointer = buffer_info.first;

    // 容纳主键的容器
    rbtree_key *new_keys = new rbtree_key;
    new_keys->reserve(primary_key_number_);

    // key的读取地址
    void *key_address = buffer_pointer;

    // 读取前面几行primary key
    for (int i = 0; i < primary_key_number_; i++)
    {
        // 读取并加入容器
        new_keys->push_back(*(primary_key_type *)key_address);

        // 移动到下一变量的读取位置
        key_address = key_address + sizeof(primary_key_type);
    }

    return pair<void *, rbtree_key *>(buffer_pointer, new_keys);
}

// 读取下一行，返回红黑树的节点pair<rbtree_key, uint32>
// 适用于宕机后从热数据文件中恢复数据，建立有序索引
inline rbtree_key *TableRowHandler::read_row_index(int row_order = -1)
{
    void *buffer;

    // 传入参数小于0，表明往下读一行
    if (row_order < 0)
    {
        // buffer_info = file_.read_lines_into_buffer(0, line_size_, true, false);
        // 弃用上述函数，现使用更加节省IO的函数：
        buffer = file_.read_primary_key_into_buffer(
            0,
            primary_key_number_ * sizeof(primary_key_type),
            true);
    }
    // 传入参数不小于0，表示读某一行
    else
    {
        // buffer_info = file_.read_lines_into_buffer(
        //     (row_order - 1) * line_size_,
        //     line_size_,
        //     1);
        // 弃用上述函数，现使用更加节省IO的函数：
        buffer = file_.read_primary_key_into_buffer(
            (row_order - 1) * line_size_,
            primary_key_number_ * sizeof(primary_key_type),
            false);
    }

    // 可用行数为0，表示读取到文件末尾了，结束了
    if (buffer == NULL)
        return NULL;

    // 容纳主键的容器
    rbtree_key *new_keys = new rbtree_key;
    new_keys->reserve(primary_key_number_);

    // key的读取地址
    void *key_address = buffer;

    // 读取前面几行primary key
    for (int i = 0; i < primary_key_number_; i++)
    {
        // 读取并加入容器
        new_keys->push_back(*(primary_key_type *)key_address);

        // 移动到下一变量的读取位置
        key_address = key_address + sizeof(primary_key_type);
    }

    // 释放
    delete[](char *) buffer;

    return new_keys;
}

inline pair<void *, uint32_t> TableRowHandler::read_buffer_and_index_to_the_end()
{
    return file_.read_lines_into_buffer(0, line_size_, 0, true, true);
}

inline void TableRowHandler::write_rows(void *buffer, row_order line_number = 1)
{
    // debug
    assert(buffer != NULL);

    // p游标总是保持在文件的末尾，因此直接写入就好了
    file_.append(buffer, line_size_ * line_number);
}

inline void *TableRowHandler::read_row_buffer(row_order line_order)
{
    // 比如要读第1行，那就从0开始；要读第2行，那就从line_size位置开始。
    pair<void *, uint32_t> buffer_info = file_.read_lines_into_buffer(
        (line_order - 1) * line_size_,
        line_size_,
        1);

    // 没有这一行
    if (buffer_info.second == 0)
        return NULL;
    else
        return buffer_info.first;
}

// 特化
template json *TableRowHandler::read_and_serialize(uint32_t &off_set, uint32_t &line_number, list<int> wanted_columns);
template list<json> *TableRowHandler::read_and_serialize(uint32_t &off_set, uint32_t &line_number, list<int> wanted_columns);

template status_code TableRowHandler::deserialize_and_write(const list<json> &rows_information);
template status_code TableRowHandler::deserialize_and_write(const json &rows_information);