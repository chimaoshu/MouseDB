#ifndef MOUSEDB_SRC_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_
#define MOUSEDB_SRC_DATA_FILE_HANDLER_DATA_FILE_HANDLER_H_

// 用于判断文件是否存在
#include <unistd.h>

// 用于内存拷贝
#include <string.h>

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include "src/exception_handler/exception_handler.h"
#include "third_part/Json/single_include/nlohmann/json.hpp"

// 文件处理类
// 使用默认构造函数，因此需要在实例化后自行调用open打开文件，再进行后续操作。
class DataFileHandler {
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
  std::map<uint32_t, void *> pages_cache_;

  // 队列，按照LRU算法，记录缓存页的页码
  std::list<uint32_t> LRU_cache_list;

  // 将指定页码的页读取到内存，并返回
  void *read_page_to_memory(uint32_t page_order);

  /**
   * @brief 传入页码，返回页面缓存
   * 如果已有缓存，则提到list最前，返回
   * 如果没有缓存且未满，则读取到内存中，放到list最前，返回
   * 如果没有缓存且已满，则读取到内存中，放到list最前，并挤掉最后一个，返回
   * 只管get就是了，缓不缓存全部是它自己管理
   *
   * @param pgae_order 第几页
   * @return void* 返回的数据，将会是取出的数据块
   */
  void *LRU_get(uint32_t pgae_order);

  /**
   * @brief 删除某一个队列在LRU队列中的缓存（如果存在的话）
   *
   * @param order_of_page_to_delete 要删除的页的序号
   */
  void LRU_delete(uint32_t order_of_page_to_delete);

public:
  /**
   * @param number_of_page_to_cache // 缓存的页数，默认为0，0表示不缓存
   */
  DataFileHandler(uint16_t number_of_page_to_cache = 0);
  ~DataFileHandler();

  /**
   * @brief 路径下的文件是否存在
   *
   * @param path 文件路径
   * @return true 文件存在
   * @return false 文件不存在
   */
  static bool file_exist(const std::string &path);

  // 获取文件路径
  std::string get_path();

  // 获取文件大小（字节）
  uint64_t get_file_size();

  /**
   * @brief 给成员变量file_赋值
   * 若不可读，那么在文件不存在时，就会创建文件
   * 若可读和可写同时出现，那么没有创建属性
   * truncate和append不能同时出现，否则文件会打开失败
   * write：可写，成假时只读
   * append：可写时从最后写
   * truncate：可写时在打开时清空文件
   * read：可读
   *
   * @param file_path 文件路径
   * @param write 可写，成假时只读
   * @param append 可写时从最后写
   * @param truncate 可写时在打开时清空文件
   * @param read 可读
   * @return StatusCode 状态码
   */
  StatusCode open(const std::string &file_path, bool write = 1, bool append = 0,
                  bool truncate = 0, bool read = 0);

  /**
   * @brief 移动游标到文件头，默认移动到文件头，适合接下来进行顺序读
   *
   * @param position 移动的目标位置（第几个字节）
   */
  void move_g_cursor(int position = 0);

  bool is_open();

  // 关闭file_，但不析构，可以重新open。
  void close();

  // 在文件末尾追加字符串，用于读JSON
  void append(const std::string &string_to_write);

  // 在文件末尾追加二进制数据，用于读mdb
  void append(void *buffer, int buffer_size);

  /**
   * @brief 从start_position开始，以每行line_size的大小，读number_of_line行
   * 若begin_at_the_current_cursor=true，则不是从start_position开始
   * 而是从上次游标的位置开始，即：不进行游标g的seek移动操作
   * 返回指向内存的指针与读取到的可用行数
   * read_to_the_end=true 直接读到文件末尾
   *
   * @param start_position 起始位置（字节）
   * @param line_size 每行大小（字节）
   * @param number_of_line 行数
   * @param begin_at_the_current_cursor
   * true则忽略给出的起始位置，直接从当前游标位置往下度
   * @param read_to_the_end 读到文件末尾
   * @return std::pair<void *, uint32_t> first是内存的指针，second是读取的行数
   */
  std::pair<void *, uint32_t> read_lines_into_buffer(
      uint64_t start_position, const int &line_size, const int &number_of_line,
      bool begin_at_the_current_cursor = false, bool read_to_the_end = false);

  /**
   * @brief 读取某一行的primary_key到内存中，不进行序列化，只读内存块
   * 如果只要读取主键信息，比起使用read_lines_into_buffer（读取整行信息）
   * 此处只读主键信息，更加节省磁盘IO，但是这里每次只读一行。
   *
   * @param start_position 起始位置（字节）
   * @param primary_keys_size 主键的大小
   * @param begin_at_the_current_cursor
   * true则忽略给出的起始位置，直接从当前游标位置往下度
   * @return void* 读取的行的内存块
   */
  void *read_primary_key_into_buffer(uint64_t start_position,
                                     const int &primary_keys_size,
                                     bool begin_at_the_current_cursor = false);

  // 读取整个文件内容，返回字符串和状态码
  // 在异常或者已读取到底的情况下，字符串为空
  respond<std::string> read_all_text();
};
#endif // MOUSEDB_SRC_DATA_FILE_HANDLER_H_