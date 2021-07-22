#ifndef MOUSEDB_SRC_TABLE_ROW_HANDLER_TABLE_ROW_HANDLER_H_
#define MOUSEDB_SRC_TABLE_ROW_HANDLER_TABLE_ROW_HANDLER_H_

// for debug
// #include <iostream>

#include "third_part/Json/single_include/nlohmann/json.hpp"

#include "src/data_file_handler/data_file_handler.h"
#include "src/table_meta_handler/table_meta_handler.h"
#include "src/table_row_handler/buffer_reader_and_writer.h"
#include "src/utils/rbtree.h"

class TableRowHandler {
    
private:
  DataFileHandler file_;

  // 表名
  const std::string table_name_;

  // 一行所占字节数
  const uint16_t line_size_;

  // 对TableMetaHandler成员指针的引用
  const std::vector<uint16_t> &off_set_of_each_column_;
  const std::vector<int8_t> &type_of_each_column_;

  // 对TableMetaHandler成员的引用
  // const nlohmann::json &table_meta_;

  // primary_key数量
  int primary_key_number_;

public:
  // 打开文件
  // 若打开失败，file.is_open() = false
  // 若truncate为true，打开时如果文件存在，那么会清空内容，适用于创建一个新文件写入的场景
  TableRowHandler(const std::string &file_path,
                  TableMetaHandler *&table_meta_handler, bool truncate,
                  uint8_t cache_pages);

  ~TableRowHandler() = default;

  // 获取文件中的行数量，冷数据文件建立索引时需要用到
  row_order get_number_of_rows_in_file();

  // 传入：偏移行数、要读行数、想要的列
  // 从第off_set+1行开始读（比如off_set=0时，从第一行开始读），读line_number行中wanted_columns
  // 其中wanted_columns的格式参考type_of_each_column_，传入的是id，不要越界
  // 返回一个JSON/Vector指针
  // 比如三列数据表示：|序号|班级号|分数|
  // 假如一共读了三个同学的数据，那么返回的数据为：
  // [
  //     [1,1,90],
  //     [2,1,100],
  //     [3,2,88]
  // ]
  // 内层是JSON，外城是JSON或vector
  // 调试用JSON、使用换Vector
  // 读取完需要释放内存
  template <class T>
  T *read_and_deserialize(uint32_t &off_set, uint32_t &line_number,
                          std::list<int> wanted_columns);

  // 读取后返回序列化为protobuf的数据
  // TODO

  // 返回状态码
  // 在表末尾追加一行或多行数据，传入的JSON格式同上（内层JSON，外层是JSON或Vector）
  // 增加多少行数据取决于JSON的第二维的数组中含有的数组的数目
  // 比如传入这个JSON就意味着增加三行
  // [
  //     [1,1,90],
  //     [2,1,100],
  //     [3,2,88]
  // ]
  // 调试用JSON、使用换Vector
  template <class T> StatusCode serialize_and_write(const T &rows_information);

  // 读取一行的数据，返回pair
  // pair第一个指针指向读取那行的内存
  // pair第二个指针指向由primary_key构成的vector的内存
  // 使用完毕需要释放内存
  // 适用于冷热数据归并时的旧冷数据文件
  std::pair<void *, rbtree_key *> read_next_row_buffer_and_index();

  // 从当前游标位置开始，一直读到文件末尾
  // 适用于冷热数据归并时的旧冷数据文件
  // 返回内存与行数
  inline std::pair<void *, uint32_t> read_buffer_and_index_to_the_end() {
    return file_.read_lines_into_buffer(0, line_size_, 0, true, true);
  }

  // 读取某行，返回红黑树的主键
  // 当row_order为负数时，表明读下一行（不进行指针移动操作），默认为该情况
  // 适用于宕机后从热数据文件中恢复数据，建立有序索引
  // 当row_order不小于0时，表明读给定行号的一行
  // 适用于冷数据建立索引时的情况
  rbtree_key *read_row_index(int row_order = -1);

  // 给定数据在第几行，直接读取那一行的数据，返回指针
  // 适用于冷热数据归并时的旧热数据文件
  void *read_row_buffer(row_order line_order);

  // 把数据写入下一行，默认写入一行
  // 适用于冷热数据归并时的新冷数据文件
  void write_rows(void *buffer, row_order line_number = 1);
};
#endif // MOUSEDB_SRC_TABLE_ROW_HANDLER_TABLE_ROW_HANDLER_H_