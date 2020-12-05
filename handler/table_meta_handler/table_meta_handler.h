#ifndef MOUSEDB_HANDLER_TABLE_META_HANDLER_TABLE_META_HANDLER_H_
#define MOUSEDB_HANDLER_TABLE_META_HANDLER_TABLE_META_HANDLER_H_

#include <string>
#include <fstream>
#include <map>
#include<list>

// json解析库
#include "lib/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "handler/data_file_handler/data_file_handler.h"
#include "handler/exception_handler/exception_handler.h"

using namespace std;

using json = nlohmann::json;

// 处理表头的类：
// 每个表头都需要实例化该类一次进行管理（即一个表头对应一个实例）
// 对表头的一切处理只能通过该类来完成
//
// 功能:
// 通过且只能通过该类的实例对表头的数据进行修改与存储与删除等操作
// 每次修改元数据，直到save才会将修改同步到磁盘，否则只会在内存中修改
//
// 注意：
// 由于其他类的table_meta均是对此类table_meta的引用
// 优点是该类对table_meta的所有更改会自适应到其他引用处
// 缺点是对于每一个实例，你需要一直保留着，直到你不再需要修改或读取元数据
class TableMetaHandler
{
private:

    // 表元数据
    json table_meta_;

    // 表元数据的存储路径
    string table_meta_path_;

    // 记录一行在磁盘中所占byte数
    // 当首次获取时会进行计算并赋值
    // 后续获取时会直接读取
    uint16_t line_size_ = 0;

    // 记录每一列对应的地址偏移，以byte为单位，用于读写时进行定位
    // 首元素存储长度
    // 当首次获取时会进行计算并赋值
    // 后续获取时会直接读取
    vector<uint16_t> off_set_of_each_column_;

    // 用数组来存储每列的key种类，首元素存储长度
    // 当首次获取时会进行计算并赋值
    // 后续获取时会直接读取    
    // 1:int_8_t
    // 2:int_16_t
    // 3:int_32_t
    // 4:int_64_t
    // 5:uint_8_t
    // 6:uint_16_t
    // 7:uint_32_t
    // 8:uint_64_t
    // 9:float
    // 10:double
    // -123:char（负数表示char，数值表示char的长度）（其实char的长度也可以通过前后偏移相减得到）
    vector<int8_t> type_of_each_column_;

    // 存储每一列的列名与对应的序号
    map<string, int> map_from_column_name_to_column_order_;

    // off_set_of_each_column_
    // type_of_each_column_
    // map_from_column_name_to_column_order_
    // 对以上三个关于行和列的信息进行计算并存储
    status_code set_column_info_and_line_size();

    // 通过类成员中的table_meta_path_读取文件数据
    status_code load();

public:

    // 以下三个函数：
    // 当首次调用时会进行计算并赋值
    // 后续调用时会直接读取
    uint16_t get_line_size();
    vector<uint16_t> &get_off_set_of_each_column();
    vector<int8_t> &get_type_of_each_column();

    const string get_table_name();

    // 获取表元数据的存储路径，只读
    const string &get_table_meta_path();

    // 获取表元数据，只读
    const json &get_table_meta();

    // TableMetaHandler() = delete;

    // 传入表头路径，通过load函数读取表头数据进行构造
    TableMetaHandler(const string &table_meta_path);

    // 传入表路径、表头数据直接构造
    TableMetaHandler(const string &table_meta_path, json &table_meta);

    // 使用传入的路径保存表头文件（另存为）
    status_code save(const string &table_meta_path);

    // 使用类成员中的表头路径保存表头文件（替换原文件内容）
    status_code save();

    // 传入列名，获取对应的列序号并从小到大排序返回
    list<int> get_column_orders_by_names(const list<string>&column_names);
};
#endif // MOUSEDB_HANDLER_TABLE_META_HANDLER_H_