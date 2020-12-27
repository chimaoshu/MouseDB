#ifndef MOUSEDB_SRC_TABLE_ROW_HANDLER_TABLE_ROW_HANDLER_H_
#define MOUSEDB_SRC_TABLE_ROW_HANDLER_TABLE_ROW_HANDLER_H_

// for debug
#include <iostream>

#include "third_part/Json/single_include/nlohmann/json.hpp"

#include "src/table_meta_handler/table_meta_handler.h"
#include "src/data_file_handler/data_file_handler.h"
#include "src/table_row_handler/buffer_reader_and_writer.h"
#include "src/hot_data/hot_data.h"


class TableRowHandler
{
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
    const nlohmann::json &table_meta_;

public:
    // 打开文件
    // 若打开失败，file.is_open() = false
    // 若truncate为true，打开时如果文件存在，那么会清空内容
    TableRowHandler(const std::string &file_path, TableMetaHandler *&table_meta_handler, bool truncate=false);

    ~TableRowHandler() = default;

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
    T *read(uint32_t &off_set, uint32_t &line_number, std::list<int> wanted_columns);

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
    template <class T>
    status_code write(const T &rows_information);
};
#endif // MOUSEDB_SRC_TABLE_ROW_HANDLER_TABLE_ROW_HANDLER_H_