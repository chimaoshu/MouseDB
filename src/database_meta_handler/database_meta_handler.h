#ifndef MOUSEDB_SRC_DATABASE_META_HANDLER_DATABASE_META_HANDLER_H_
#define MOUSEDB_SRC_DATABASE_META_HANDLER_DATABASE_META_HANDLER_H_

#include<sys/stat.h>

#include <string>

#include "third_part/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "src/data_file_handler/data_file_handler.h"

// 异常处理
#include "src/exception_handler/exception_handler.h"

#include "src/tools/tools.h"


// 负责数据库元数据的读取、增删、修改与保存
// 所有其他处出现的数据库元数据JSON对象均来自对该类成员的引用
// 因此除非你确定以后不会再对meta信息进行读写操作，那么你不应该删除实例
// 该类实例不应该被释放，直到程序结束
class DatabaseMetaHandler
{
private:
    // 数据库元数据JSON文件的存储路径
    // 这个路径是固定的，不会由用户指定
    // 这些数据的存储对用户是透明的
    std::string database_meta_path_;

    // 数据库元数据
    // 格式参考format/database_meta.jsonc
    nlohmann::json database_meta_;

    // 通过类成员中的文件路径去读取元数据
    // 当文件读取失败时，会抛出异常
    // 当JSON格式不合法时，会返回错误码
    status_code load();

public:
    // 返回数据库元数据JSON文件的存储路径
    const std::string &get_database_meta_path();

    // 以JSON类的格式返回数据库元数据的引用
    const nlohmann::json &get_database_meta();

    // 为了功能解耦的严格性，对数据库元数据的一切操作都应该由该类完成，因此不再使用set
    // void set_database_meta_information(const json &database_meta);

    // 传入数据库元数据JSON文件的路径，通过load获取到元数据
    // 若所给路径不存在文件，则创建
    DatabaseMetaHandler(const std::string &database_meta_dir);

    // 传入数据库元数据JSON文件路径和元数据，直接赋值
    DatabaseMetaHandler(const std::string &database_meta_dir, nlohmann::json &database_meta);

    // 覆盖原来文件保存
    status_code save();

    // 另存为其他路径下的文件
    status_code save(const std::string &database_meta_path);

    // 在元数据中加入一个数据库，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    status_code add_database_in_json(const std::string &database_name);

    // 在元数据中加入一个表，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    status_code add_table_in_json(const std::string &database_name, const std::string &table_name, const std::string &table_dir);

    // 在元数据中删除一个数据库，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    status_code delete_database_in_json(const std::string &database_name);

    // 在元数据中删除一个表，并返回状态码
    // 在使用save()函数保存之间，数据不会被存储到磁盘中
    status_code delete_table_in_json(const std::string &database_name, const std::string &table_name);
};

#endif // MOUSEDB_SRC_DATABASE_META_HANDLER_DATABASE_META_HANDLER_H_