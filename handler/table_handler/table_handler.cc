#include "table_handler.h"

#include <string>
#include <iostream>

// 删除文件
#include <cstdio>

#include "handler/data_file_handler/data_file_handler.h"

TableHandler::TableHandler(const string database_name, const json &database_meta)
{
    database_name_ = database_name;
    database_meta_ = database_meta;
}

respond<string> TableHandler::get_table_dir(const string &table_name)
{
    // 通过JSON获取
    auto it = database_meta_[database_name_].find(table_name);
    if (it != database_meta_[database_name_].end())
        return respond<string>(it.key(),error_code::SUCCESS);
    else
        return respond<string>("", error_code::ERROR_TABLE_INFO_NOT_FOUND);
}

status_code TableHandler::create(const string &table_directory, const string &table_name, const list<column *> &columns, DatabaseMetaHandler &db_mete_handler)
{
    // TODO：检查路径合法性（应以'/'结尾，且不包含特殊字符）
    // check(table_directory);

    // 存入路径与表名
    tables_in_database_[table_name]["storage_path"] = table_directory;
    tables_in_database_[table_name]["table_name"] = table_name;

    // 遍历每一个column，转化为JSON存入
    for (list<column *>::const_iterator i = columns.begin(); i != columns.end(); i++)
    {
        column *each_column = *i;

        // 代表某列数据的JsonObject
        json each_column_in_json;

        each_column_in_json["name"] = each_column->key_name;
        each_column_in_json["type"] = each_column->type;
        each_column_in_json["not_null"] = each_column->not_null;

        if (each_column->is_primary)
            tables_in_database_[table_name]["primary_key"] = each_column_in_json;
        else
            tables_in_database_[table_name]["other_keys"].push_back(each_column_in_json);
    }

    // 保存表头JSON到磁盘
    string table_meta_path = table_directory + table_name + ".json";
    TableMetaHandler table_meta_ = TableMetaHandler(table_meta_path, tables_in_database_);
    table_meta_.save();

    // 创建表文件
    string table_path = table_directory + table_name + ".mdb";
    DataFileHandler table;
    table.open(table_path, 1, 0, 0, 0);

    // 在数据库元数据JSON中增加该表信息
    status_code err = db_mete_handler.add_table_in_json(database_name_, table_name, table_directory);

    // 保存到磁盘
    db_mete_handler.save();

    // 返回状态码
    return err;
}

status_code TableHandler::drop_table(const string &table_name, DatabaseMetaHandler &db_mete_handler)
{
    respond<string> table_dir = get_table_dir(table_name);

    // 检查错误码
    if (table_dir.second)
        return table_dir.second;

    // 先删除二进制文件
    remove((table_dir.first + table_name + ".mdb").c_str());

    // 然后删除表头
    remove((table_dir.first + table_name + ".json").c_str());

    // 调用DatabaseMetaHandler来抹去JSON上该表数据
    status_code error = db_mete_handler.delete_table_in_json(database_name_, table_name);

    // 保存到磁盘
    db_mete_handler.save();

    // 返回状态码
    return error;
}

vector<string> *TableHandler::get_all_table_names()
{
    vector<string> *all_table_names = new vector<string>;

    // 获取所有key
    for (auto it = database_meta_[database_name_].begin(); it != database_meta_[database_name_].end(); it++)
    {
        all_table_names->push_back(it.key());
    }

    return all_table_names;
}