#ifndef MOUSEDB_SRC_DATABASE_HANDLER_DATABASE_HANDLER_H_
#define MOUSEDB_SRC_DATABASE_HANDLER_DATABASE_HANDLER_H_


#include <string>

// 数据库元数据处理
#include "src/database_meta_handler/database_meta_handler.h"

// 当你打开一个数据库，将返回一个table_handler实例
#include "src/table_handler/table_handler.h"

// 异常处理
#include "src/exception_handler/exception_handler.h"

// 管理元数据中描述的所有的数据库，如打开、创建、删除数据库等操作
// 元数据格式请参考format/database_meta.jsonc
// 该类实例不应该被释放，直到程序结束
class DatabaseHandler
{
private:
    // 虽然DatabaseHandler中很多操作需要用到DatabaseMetaHandler
    // 但是考虑到功能解耦的优越性，决定不在该类中实例化DatabaseMetaHandler
    // 只在类成员中保存对DatabaseMetaHandler类实例的引用
    DatabaseMetaHandler &database_meta_handler_;

    // 数据库元数据
    // 是对DatabaseMetaHandler类成员的引用
    const nlohmann::json &database_meta_;

    // 数据库元数据所在目录
    // 是对UserAction类成员的引用
    std::string &database_meta_dir_;

public:
    // 传入对DatabaseMetaHandler类实例的引用
    DatabaseHandler(DatabaseMetaHandler &database_meta_handler, std::string &database_meta_dir);

    // 创建一个数据库并返回状态码
    // 将调用DatabaseMetaHandler实例对JSON进行修改，并创建一个目录
    status_code create(const std::string &database_name);

    // 打开一个数据库，返回指向TablesHandler实例的指针
    // 若指针为空，则打开失败
    TablesHandler *open(const std::string &database_name);

    // 删除一个数据库并返回状态码
    // 过程包括：删除数据库中的表、表头，删除数据库元数据中内容
    status_code drop(const std::string &database_name);

    // 返回所有数据库的名字
    std::list<std::string> get_all_database_name();
};

#endif // MOUSEDB_SRC_DATABASE_HANDLER_DATABASE_HANDLER_H_