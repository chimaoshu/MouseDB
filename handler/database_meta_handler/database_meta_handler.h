#ifndef MOUSEDB_HANDLER_DATABASE_META_HANDLER_DATABASE_META_HANDLER_H_
#define MOUSEDB_HANDLER_DATABASE_META_HANDLER_DATABASE_META_HANDLER_H_

#include <string>

#include "lib/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "handler/data_file_handler/data_file_handler.h"

// 异常处理
#include "handler/exception_handler/exception_handler.h"

using namespace std;
using json = nlohmann::json;

// 负责数据库元数据的读取、增删、修改与保存
// 所有其他处出现的数据库元数据JSON对象均来自对该类成员的引用
// 不负责元数据JSON对象的某项属性的查询任务，只负责返回JSON对象的引用
class DatabaseMetaHandler
{
private:
    // 数据库元数据JSON文件的存储路径
    // 这个路径是固定的，不会由用户指定
    // 这些数据的存储对用户是透明的
    string database_meta_path_;

    // 数据库元数据
    // 格式参考format/database_meta.jsonc
    json database_meta_;

    // 通过类成员中的文件路径去读取元数据
    // 当文件读取失败时，会抛出异常
    // 当JSON格式不合法时，会返回错误码
    void load();

public:
    // 返回数据库元数据JSON文件的存储路径
    const string &get_database_meta_path();

    // 以JSON类的格式返回数据库元数据的引用
    const json &get_database_meta();

    // 对数据库元数据的一切操作都应该由该类完成，因此不再使用set
    // void set_database_meta_information(const json &database_meta);

    // 传入数据库元数据JSON文件的路径，通过load获取到元数据
    DatabaseMetaHandler(const string &database_meta_path);

    // 传入数据库元数据JSON文件路径和元数据，直接赋值
    DatabaseMetaHandler(const string &database_meta_path, json &database_meta);

    // 覆盖原来文件保存
    // 当文件保存失败时，抛出异常
    void save();

    // 另存为其他路径下的文件
    // 当文件保存失败时，抛出异常
    void save(const string &database_meta_path);

    // 在元数据中加入一个数据库，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    // 数据库已存在时，返回ERROR_DATABASE_HAS_ALREADY_EXISTS
    status_code add_database_in_json(const string &database_name);

    // 在元数据中加入一个表，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    // 表已存在时，返回ERROR_TABLE_NOT_EXISTS
    status_code add_table_in_json(const string &database_name, const string &table_name, const string &table_dir);

    // 在元数据中删除一个数据库，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    // 数据库不存在时，返回ERROR_DATABASE_NOT_EXISTS
    status_code delete_database_in_json(const string &database_name);

    // 在元数据中删除一个表，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    // 表不存在时，返回ERROR_TABLE_HAS_ALREADY_EXISTS
    status_code delete_table_in_json(const string &database_name, const string &table_name);
};

#endif // MOUSEDB_HANDLER_DATABASE_META_HANDLER_DATABASE_META_HANDLER_H_