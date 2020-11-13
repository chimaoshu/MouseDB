#include "table_meta_handler.h"

const string TableMetaHandler::get_table_name()
{
    return table_meta_information_["table_name"];
}

const string &TableMetaHandler::get_table_meta_path()
{
    return table_meta_path_;
}

const json &TableMetaHandler::get_table_meta_information()
{
    return table_meta_information_;
}

TableMetaHandler::TableMetaHandler(const string &table_meta_path)
{
    // TODO：路径检查：非空、特殊字符....
    table_meta_path_ = table_meta_path;
    load(); // 通过load读取对应路径下元数据
}

TableMetaHandler::TableMetaHandler(const string &table_meta_path, json &table_meta)
{
    // TODO：路径检查：非空、特殊字符....
    table_meta_path_ = table_meta_path;
    table_meta_information_ = table_meta; // 直接传入元数据
}

void TableMetaHandler::save(const string &table_meta_path)
{
    DataFileHandler file;
    file.open(table_meta_path, 1, 0, 1, 0);
    file.append(table_meta_information_.dump());
}

void TableMetaHandler::save()
{
    // 在构造函数加入路径合法性检查后，就可以删除以下判断了
    // if (table_meta_information_["table_name"].empty() || table_meta_path_.empty() || table_meta_information_.empty())
    //     throw "TableMetaHandler::save()：未初始化！";

    DataFileHandler file;
    file.open(table_meta_path_, 1, 0, 1, 0);

    file.append(table_meta_information_.dump());
}

void TableMetaHandler::load()
{
    // if (!table_meta_information_.empty())
    //     throw "TableMetaHandler::load()：不能重复读取";

    DataFileHandler file;
    file.open(table_meta_path_, 0, 0, 0, 1);

    pair<string, status_code> p = file.read_all();

    if (!p.first.empty())
        table_meta_information_ = json::parse(p.first);
    else
        error_code::check_error(p.second);
}
