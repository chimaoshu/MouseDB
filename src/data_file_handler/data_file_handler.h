#ifndef MOUSEDB_SRC_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_
#define MOUSEDB_SRC_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_

// 用于判断文件是否存在
#include <unistd.h>

// 用于内存拷贝
#include <string.h>

#include <map>
#include <list>
#include <string>
#include <fstream>
#include <iostream>

#include "src/exception_handler/exception_handler.h"
#include "third_part/Json/single_include/nlohmann/json.hpp"

// 文件处理类
// 使用默认构造函数，因此需要在实例化后自行调用open打开文件，再进行后续操作。
class DataFileHandler
{
private:
    // 路径
    std::string path_;

    // 打开模式
    bool writable_;
    bool readable_;
    bool appendable_;
    bool truncate_;

    // 被打开的文件
    std::fstream file_;

    // 文件大小
    uint64_t file_size_;

    // 缓存页数的最大限制
    uint16_t number_of_page_to_cache_;

    // 以下的类成员和类方法，均和缓存有关

    // 一页大小为4k
    const int page_size_ = 4096;

    // 从页码到对应内存块的映射
    std::map<uint32_t, void*> pages_cache_;

    // 队列，按照LRU算法，记录缓存页的页码
    std::list<uint32_t> LRU_cache_list;

    // 将指定页码的页读取到内存，并返回
    void *read_page_to_memory(uint32_t page_order);

    // 传入页码，返回页面缓存
    // 如果已有缓存，则提到list最前，返回
    // 如果没有缓存且未满，则读取到内存中，放到list最前，返回
    // 如果没有缓存且已满，则读取到内存中，放到list最前，并挤掉最后一个，返回
    // 只管get就是了，缓不缓存全部是它自己管理
    void *LRU_get(uint32_t pgae_order);

    // 删除某一个队列在LRU队列中的缓存（如果存在的话）
    void LRU_delete(uint32_t order_of_page_to_delete);

public:
    // 缓存的页数，默认为0，不缓存为0
    DataFileHandler(uint16_t number_of_page_to_cache=0); 
    ~DataFileHandler();

    // 路径下的文件是否存在
    bool file_exist(const std::string &path);

    // 返回文件路径
    std::string get_path();

    // 获取文件大小s
    inline uint64_t get_file_size();

    // 给成员变量file_赋值
    // 若不可读，那么在文件不存在时，就会创建文件
    // 若可读和可写同时出现，那么没有创建属性
    // truncate和append不能同时出现，否则文件会打开失败
    // write：可写，成假时只读
    // append：可写时从最后写
    // truncate：可写时在打开时清空文件
    // read：可读
    status_code open(const std::string &file_path, bool write = 1, bool append = 0, bool truncate = 0, bool read = 0);

    // 移动游标到文件头，适合接下来进行顺序读
    void move_g_cursor_to_the_beginning();

    bool is_open();

    // 关闭file_，但不析构，可以重新open。
    void close();

    // 在文件末尾追加字符串，用于读JSON
    status_code append(const std::string &string_to_write);

    // 在文件末尾追加二进制数据，用于读mdb
    status_code append(void *buffer, int buffer_size);

    // 从initial_adress开始，以每行line_size的大小，读number_of_line行
    // 若begin_at_the_current_cursor=true，则不是从initial_adress开始
    // 而是从上次游标的位置开始，即：不进行游标g的seek移动操作
    // 返回指向内存的指针与读取到的可用行数
    // read_to_the_end=true 直接读到文件末尾
    std::pair<void *, uint32_t> read_lines_into_buffer(
        uint64_t start_position,
        const int &line_size,
        const int &number_of_line,
        bool begin_at_the_current_cursor=false,
        bool read_to_the_end=false);

    // 读取一行字符串，返回字符串和状态码
    // 在异常或者已读取到底的情况下，字符串为空
    respond<std::string> read_line();

    // 读取整个文件内容，返回字符串和状态码
    // 在异常或者已读取到底的情况下，字符串为空
    respond<std::string> read_all();
};
#endif // MOUSEDB_SRC_DATA_FILE_HANDLER_H_