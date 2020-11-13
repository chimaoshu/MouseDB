#ifndef MOUSEDB_HANDLER_TABLE_META_HANDLER_TABLE_META_HANDLER_H_
#define MOUSEDB_HANDLER_TABLE_META_HANDLER_TABLE_META_HANDLER_H_

#include <stdexcept>
#include <string>
#include <fstream>

namespace table_meta_handler_status_code
{
    // TODO

} // namespace table_meta_handler_status_code

// json解析库
#include "lib/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "handler/data_file_handler/data_file_handler.h"

using namespace std;

typedef int8_t status_code;

// for convenience
using json = nlohmann::json;

// 处理表头的类：
// 每个表头都需要实例化该类一次进行管理（即一个表头对应一个实例）
// 对表头的一切处理只能通过该类来完成
//
// 功能:
// 通过该类的实例对表头的数据进行修改与存储与删除等操作
// 每次修改元数据，直到save才会将修改同步到磁盘，相当于一次commit
//
// 注意：
// 对于每一个实例，你可以一直保留着，直到你不需要再使用到元数据
// 需要注意的是，由于其他类使用引用读取表元数据，所以你不能释放实例直到你决定不再读取表头数据
class TableMetaHandler
{
private:
    // 表元数据的存储路径
    string table_meta_path_;

    // 表元数据
    json table_meta_information_;

    // 通过类成员中的表头路径读取表元数据
    void load();

public:
    // 获取表名
    // 如果json解析库抛出异常，那么说明json合法性检查函数出现问题
    const string get_table_name();

    // 获取表元数据的存储路径，只读
    const string &get_table_meta_path();

    // 获取表元数据，只读
    const json &get_table_meta_information();

    // 通过以下两个函数初始化的方式不安全，已被弃用
    // TableMetaHandler() = default;
    // void set_table_meta_path(const string &table_meta_path);

    // 传入表头路径，通过load函数读取表头数据
    TableMetaHandler(const string &table_meta_path);

    // 传入表路径、表头数据的方式创建
    TableMetaHandler(const string &table_meta_path, json &table_meta);

    // 使用传入的路径保存表头文件（另存为）
    void save(const string &table_meta_path);

    // 使用类成员中的表头路径保存表头文件（即替换原文件内容）
    void save();

    // TODO：修改表元数据的函数，比如重命名，改属性之类的
};
#endif // MOUSEDB_HANDLER_TABLE_META_HANDLER_H_