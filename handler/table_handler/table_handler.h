#ifndef MOUSEDB_HANDLER_TABLE_HANDLER_TABLE_HANDLER_H_
#define MOUSEDB_HANDLER_TABLE_HANDLER_TABLE_HANDLER_H_

#include <string>
#include <list>

#include "lib/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "handler/data_file_handler/data_file_handler.h"

// 表元数据管理
#include "handler/table_meta_handler/table_meta_handler.h"

// 库元数据管理
#include "handler/database_meta_handler/database_meta_handler.h"

using namespace std;
using json = nlohmann::json;

// 元数据中的一列的定义
struct column
{
    string key_name;
    string type;
    bool is_primary;
    bool not_null;
};

// 对某个数据库中表的处理，包括表的创建、打开、删除
// TableHandler连接起了表元数据与数据库元数据库
// 与TableMetaHandler不同的是，该类管理某个数据库中的所有表
// 即一个实例对应一个被打开的数据库，用户进入一个数据库时，一个TableHandler将会被实例化
class TableHandler
{
private:
    // 数据库头JSON，存储了所有的数据库和对应的表及其路径
    // 具体格式参考format/database_meta.jsonc
    json database_meta_;

    // 存储从表名到表头JSON对象的映射
    // 每次打开一个表头JSON文件都存储一对
    // JSON对象的具体格式参考format/table_meta.jsonc
    json tables_in_database_;

    // 当前打开的数据库名（由于一个该类的实例对应一个被打开的数据库）
    string database_name_;

public:
    // 构造函数，传入数据库元数据的JSON信息（databse_meta）
    // 即所有数据库及其表信息
    // 按理说实例只要知道自己所管理的数据库的信息就够了，不需要知道别的数据库的信息
    // 但是这么做可以通过引用，可以在JSON修改（由DatabaseMetaHandler完成）时
    // 使引用了该JSON的实例的类成员也同步得到修改
    TableHandler(const string database_name_, const json &database_meta);

    // 该构造函数只用于在检查不通过时，返回一个没用的实例
    // 因此不允许在其他条件下调用
    TableHandler();

    // 传入一个表名、获取这个表名对应的表的存储路径
    // 若表不存在，返回空字符串
    respond<string> get_table_dir(const string &table_name);

    // 创建一个表
    // table_directory：表的存储路径
    // table_name：表名
    // columns：各列信息
    status_code create(const string &table_directory, const string &table_name, const list<column *> &columns, DatabaseMetaHandler &db_mete_handler);

    // 传入表名与DatabaseMetaHandler
    // 删除一个表，所做的工作包括：
    // 在database_meta中抹除该表信息并保存、在磁盘上删除对应表路径下的JSON与MDB文件
    status_code drop_table(const string &table_name, DatabaseMetaHandler &db_mete_handler);

    // 获取所有表的表名
    // 返回一个vector<string>的指针，使用完成需要释放
    vector<string> *get_all_table_names();

    // open一个表的过程不在此处实现，由TableRow自己去操作
};

#endif // MOUSEDB_HANDLER_TABLE_HANDLER_H_