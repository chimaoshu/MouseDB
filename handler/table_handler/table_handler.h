#ifndef MOUSEDB_HANDLER_TABLE_HANDLER_TABLE_HANDLER_H_
#define MOUSEDB_HANDLER_TABLE_HANDLER_TABLE_HANDLER_H_

#include <string>
#include <list>
#include <map>

#include "lib/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "handler/data_file_handler/data_file_handler.h"

// 表元数据管理
#include "handler/table_meta_handler/table_meta_handler.h"

// 库元数据管理
#include "handler/database_meta_handler/database_meta_handler.h"

using namespace std;
using json = nlohmann::json;

// 对某个数据库中表的处理，包括表的创建、打开、删除
// TableHandler连接起了表元数据与数据库元数据库
// 与TableMetaHandler不同的是，该类管理某个数据库中的所有表
// 即一个TableHandler实例将对应一个被打开的数据库
// 用户进入一个数据库时，一个TableHandler将会被实例化
// 该类不应该被释放，除非您确定以后不会再使用该数据库
// 即使暂时不使用该数据库，也建议（在UserStatusHandler中？或者DBHandler中？）用一个MAP把数据库名与该实例缓存起来
class TableHandler
{
private:
    // 数据库头JSON，存储了所有的数据库和对应的表及其路径
    // 来自对DatabaseMetaHandler成员的引用
    // 格式参考format/database_meta.jsonc
    const json &database_meta_;

    // 当前打开的数据库名（由于一个该类的实例对应一个被打开的数据库）
    string database_name_;

    // 存储从表名到table_meta_handler的映射
    // 每次创建TableHandler都会在此处缓存所有表对应的TableMetaHandler，方便读操作
    // 内容包括：表名及其对应元数据
    // 格式参考format/tables_info.jsonc
    map<string, TableMetaHandler*> map_of_name_and_table_meta_handler_;

public:
    // 构造函数，传入数据库元数据的JSON信息（databse_meta）
    // 即所有数据库及其表信息
    // 按理说实例只要知道自己所管理的数据库的信息就够了，不需要知道别的数据库的信息
    // 但是这么做可以通过引用，可以在JSON修改（由DatabaseMetaHandler完成）时
    // 使引用了该JSON的实例的类成员也同步得到修改
    TableHandler(const string database_name_, const json &database_meta);

    // 获得数据库名
    const string &get_database_name();

    // 传入表名、返回对应的table_meta的引用
    // 若不存在，返回NULL
    const json &get_table_meta(const string &table_name);

    // 返回对TableMetaHandler的引用
    TableMetaHandler *&get_table_meta_handler(const string &table_name);

    // 传入一个表名、获取这个表名对应的表的存储路径
    // 若表不存在，返回空字符串
    string get_table_dir(const string &table_name);

    // 创建一个表
    // table_directory：表的存储路径
    // table_name：表名
    // table_meta：表头
    // db_mete_handler：对数据库元数据进行操作
    status_code create(const string &table_directory, const string &table_name, json &table_meta, DatabaseMetaHandler &db_mete_handler);

    // 传入表名与DatabaseMetaHandler
    // 删除一个表，所做的工作包括：
    // 在database_meta中抹除该表信息并保存、在磁盘上删除对应表路径下的JSON与MDB文件
    status_code drop_table(const string &table_name, DatabaseMetaHandler &db_mete_handler);

    // 返回该数据库下所有表的表名
    list<string> get_all_table_names();
};

#endif // MOUSEDB_HANDLER_TABLE_HANDLER_H_