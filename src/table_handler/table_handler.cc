#include "table_handler.h"


using namespace std;
using json = nlohmann::json;

TablesHandler::TablesHandler(const string &database_name, const json &database_meta)
    : database_meta_(database_meta)
{
    database_name_ = database_name;

    string table_dir, table_name;

    // 生成每一个表的TableMetaHandler并赋值到map中
    for (auto it = database_meta[database_name].begin(); it != database_meta[database_name].end(); it++)
    {
        table_name = it.key();
        table_dir = it.value();

        string table_meta_path = table_dir + '/' + table_name + ".json";
        map_of_name_and_table_meta_handler_[table_name] = new TableMetaHandler(table_meta_path);
    }
}

// 释放TeblaMetaHandler
TablesHandler::~TablesHandler()
{
    // 遍历map_of_name_and_table_meta_handler_
    for (auto it = map_of_name_and_table_meta_handler_.begin(); it != map_of_name_and_table_meta_handler_.end(); it++)
    {
        // 释放TeblaMetaHandler
        if (it->second)
            delete it->second;
    }
}

const string &TablesHandler::get_database_name() { return database_name_; }

string TablesHandler::get_table_dir(const string &table_name)
{
    // 遍历database_meta_获取
    auto it = database_meta_[database_name_].find(table_name);
    if (it != database_meta_[database_name_].end())
        return it.value();
    else
        return "";
}

status_code TablesHandler::create(const string &table_directory, const string &table_name, json &table_meta, DatabaseMetaHandler &db_mete_handler)
{
    // 创建表文件
    string table_path = table_directory + '/' + table_name + ".mdb";
    DataFileHandler table;
    table.open(table_path, 1, 0, 0, 0);

    // 在数据库元数据JSON中增加该表信息并保存到磁盘
    db_mete_handler.add_table_in_json(database_name_, table_name, table_directory);
    db_mete_handler.save();

    // 更新map_of_name_and_table_meta_handler_
    string table_meta_path = table_directory + '/' + table_name + ".json";
    map_of_name_and_table_meta_handler_[table_name] = new TableMetaHandler(table_meta_path, table_meta);

    // 保存表头JSON到磁盘，并返回状态码
    return map_of_name_and_table_meta_handler_[table_name]->save();
}

status_code TablesHandler::drop_table(const string &table_name, DatabaseMetaHandler &db_mete_handler)
{
    string table_dir = get_table_dir(table_name);

    // 先删除二进制文件
    remove((table_dir + '/' + table_name + ".mdb").c_str());

    // 然后删除表头
    remove((table_dir + '/' + table_name + ".json").c_str());

    // 删除文件夹
    remove(table_dir.c_str());

    // 调用DatabaseMetaHandler来抹去JSON上该表数据
    status_code error = db_mete_handler.delete_table_in_json(database_name_, table_name);

    // 保存到磁盘
    db_mete_handler.save();

    // 删除tables_info中的表头数据
    auto it = map_of_name_and_table_meta_handler_.find(table_name);
    if (it != map_of_name_and_table_meta_handler_.end())
        map_of_name_and_table_meta_handler_.erase(it);

    // 返回状态码
    return error;
}

list<string> TablesHandler::get_all_table_names()
{
    list<string> all_table_name;

    // 遍历map_of_name_and_table_meta_handler的keys
    for (auto it = map_of_name_and_table_meta_handler_.begin(); it != map_of_name_and_table_meta_handler_.end(); it++)
    {
        all_table_name.push_back(it->first);
    }
    return all_table_name;
}

const json &TablesHandler::get_table_meta(const string &table_name)
{
    // 取得表名对应的TableMetaHandler实例
    auto it = map_of_name_and_table_meta_handler_.find(table_name);

    if (it != map_of_name_and_table_meta_handler_.end())
        // 通过TableMetaHandler实例获得表头信息
        return it->second->get_table_meta();
    else
        return NULL;
}

TableMetaHandler *&TablesHandler::get_table_meta_handler(const string &table_name)
{
    // 返回表名对应的指向TableMetaHandler实例的指针的引用
    return map_of_name_and_table_meta_handler_[table_name];
}