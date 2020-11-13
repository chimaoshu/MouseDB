#include <string>

// 数据库元数据处理
#include "handler/database_meta_handler/database_meta_handler.h"

// 当你打开一个数据库，将返回一个table_handler实例
#include "handler/table_handler/table_handler.h"

// 异常处理
#include "handler/exception_handler/exception_handler.h"

// 管理元数据中描述的所有的数据库，元数据格式请参考format/database_meta.jsonc
class DatabaseHandler
{
private:
    // 数据库元数据JSON文件的存储路径
    string database_meta_path_;

    // 数据库元数据
    json database_meta_;

    // 处理数据库元数据的实例
    DatabaseMetaHandler database_meta_handler_;

public:
    // 传入数据库元数据JSON文件的路径和JSON对象
    DatabaseHandler(const string &database_meta_path);

    // 创建一个数据库并返回状态码
    // 将调用MetaHandler对JSON进行修改，仅此而已
    status_code create(const string &database_name);

    // 打开一个数据库并返回TableHandler对象与状态码
    respond<TableHandler> open(const string &database_name);

    // 删除一个数据库并返回状态码
    // 过程：删除数据库中的表、表头，删除数据库元数据中内容
    status_code drop(const string &database_name);

    // TODO
    vector<string> get_all_database_name();
};

// 此处需要指定类成员database_meta_handler_的初始化
// DatabaseMetaHandler中的机制已保证此处的database_meta_handler_是正确完成初始化的
DatabaseHandler::DatabaseHandler(const string &database_meta_path)
    : database_meta_handler_(database_meta_path)
{
    database_meta_path_ = database_meta_path;
    database_meta_ = database_meta_handler_.get_database_meta();
}

status_code DatabaseHandler::create(const string &database_name)
{
    status_code error = database_meta_handler_.add_database_in_json(database_name);
    return error;
}

respond<TableHandler> DatabaseHandler::open(const string &database_name)
{
    auto it = database_meta_.find(database_name);

    // 数据库不存在
    if (it == database_meta_.end())
        return respond<TableHandler>(TableHandler(), error_code::ERROR_DATABASE_NOT_EXISTS);

    else
        // 实例化返回一个TableHandler实例，并返回成功的状态码
        return respond<TableHandler>(TableHandler(database_name, database_meta_), error_code::SUCCESS);
}

status_code DatabaseHandler::drop(const string &database_name)
{
    auto it = database_meta_.find(database_name);
    if (it == database_meta_.end())
    {
        return error_code::ERROR_DATABASE_NOT_EXISTS;
    }
    else
    {
        status_code error;

        // 实例化一个数据处理类
        TableHandler table_handler = TableHandler(database_name, database_meta_);

        // 遍历每一个表
        for (auto it = database_meta_[database_name].begin(); it != database_meta_[database_name].end(); it++)
        {
            // 删除表的表头、表数据以及在数据库元数据中的表记录
            error = table_handler.drop_table(it.key(), database_meta_handler_);
            if (error)
                return error;
        }

        // 删除JSON中的数据库
        error = database_meta_handler_.delete_database_in_json(database_name);
        if (error)
            return error;
        else
            return error_code::SUCCESS;
    }
}