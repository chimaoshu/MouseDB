#include "database_handler.h"


using namespace std;
using json = nlohmann::json;

// 此处需要指定类成员database_meta_handler_的初始化
// DatabaseMetaHandler中的机制已保证此处的database_meta_handler_是正确完成初始化的
DatabaseHandler::DatabaseHandler(DatabaseMetaHandler &database_meta_handler, string &database_meta_dir)
    : database_meta_handler_(database_meta_handler),
      database_meta_(database_meta_handler.get_database_meta()),
      database_meta_dir_(database_meta_dir)
{
}

status_code DatabaseHandler::create(const string &database_name)
{
    string database_dir = database_meta_dir_ + '/' + database_name;
    if (!access(database_dir.c_str(), 0) == 0)
        mkdir(database_dir.c_str(), S_IRWXU);

    database_meta_handler_.add_database_in_json(database_name);
    return database_meta_handler_.save();
}

TablesHandler *DatabaseHandler::open(const string &database_name)
{
    auto it = database_meta_.find(database_name);

    // 数据库不存在
    if (it == database_meta_.end())
    {
        return NULL; 
    }
    else
    {
        TablesHandler *table_handler = new TablesHandler(database_name, database_meta_);
        return table_handler;
    }
}

status_code DatabaseHandler::drop(const string &database_name)
{
    // 判断数据库是否存在
    auto i = database_meta_.find(database_name);
    if (i == database_meta_.end())
    {
        return error_code::ERROR_DATABASE_NOT_EXISTS;
    }
    else
    {
        // 实例化一个数据处理类
        TablesHandler table_handler = TablesHandler(database_name, database_meta_);

        // 遍历每一个表
        for (auto it = database_meta_[database_name].begin(); it != database_meta_[database_name].end(); it++)
        {
            // 删除表的表头、表数据以及在数据库元数据中的表记录
            table_handler.drop_table(it.key(), database_meta_handler_);
        }

        // 删除以数据库名命名的文件夹
        remove((database_meta_dir_ + '/' + database_name).c_str());

        // 删除JSON中的数据库并返回错误码
        database_meta_handler_.delete_database_in_json(database_name);
        return database_meta_handler_.save();
    }
}

list<string> DatabaseHandler::get_all_database_name()
{
    list<string> all_database_name;
    for (auto it = database_meta_.begin(); it != database_meta_.end(); it++)
    {
        all_database_name.push_back(it.key());
    }
    return all_database_name;
}