#include "handler/table_meta_handler.h"

#include <iostream>
#include <stdexcept>

// json解析库
#include "storage/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "handler/data_file_handler.h"

using namespace std;

// 通过传入json的方式创建
TableMeta::TableMeta(const string &file_storage_path, const string &table_name, json &keys_information)
{
    set_table_name(table_name);
    set_file_storage_path(file_storage_path);
    set_keys_information(keys_information);
}

// 通过load的方式创建
TableMeta::TableMeta(const string &file_storage_path, const string &table_name)
{
    set_table_name(table_name);
    set_file_storage_path(file_storage_path);
    load();
}

const string &TableMeta::get_table_name()
{
    return table_name_;
}

const string &TableMeta::get_file_storage_path()
{
    return file_storage_path_;
}

const json &TableMeta::get_keys_information()
{
    return keys_information_;
}

void TableMeta::set_table_name(const string &table_name)
{
    table_name_ = table_name;
}

void TableMeta::set_file_storage_path(const string &file_storage_path)
{
    file_storage_path_ = file_storage_path;
}

void TableMeta::set_keys_information(const json &keys_information)
{
    keys_information_ = keys_information;
}

// 使用传入的路径
void TableMeta::save(const string&path)
{
    if (table_name_.empty() || path.empty() || keys_information_.empty())
        throw "TableMeta::save()：类成员为空！";

    DataFileHandler file;
    file.open(path, 1, 0, 1, 0);

    if (!file.is_open())
        throw("TableMeta::save():文件没有打开，检查打开模式！");

    file.append(keys_information_.dump());
}

// 使用内置的路径（文件必然存在）
void TableMeta::save()
{
    if (table_name_.empty() || file_storage_path_.empty() || keys_information_.empty())
        throw "TableMeta::save()：类成员为空！";

    DataFileHandler file;
    file.open(file_storage_path_, 1, 0, 1, 0);

    if (!file.is_open())
        throw("TableMeta::save():文件没有打开，检查打开模式！");

    file.append(keys_information_.dump());
}

void TableMeta::load()
{
    if (file_storage_path_.empty())
        throw "TableMeta::load()：路径为空！";

    DataFileHandler file;
    file.open(file_storage_path_, 0, 0, 0, 1);

    if (!file.is_open())
        throw "TableMeta::load()：路径不存在！";

    string json_string = file.read_all();
    keys_information_ = json::parse(json_string);
}