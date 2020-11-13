#include "database_meta_handler.h"

DatabaseMetaHandler::DatabaseMetaHandler(const string &database_meta_path, json &database_meta)
{
    // TODO：path检查（路径是否合法）
    // 如果不合法直接抛出异常，因为此处路径不由用户决定，如果出现异常一定是代码的问题
    
    database_meta_path_ = database_meta_path;
    database_meta_ = database_meta;
}

DatabaseMetaHandler::DatabaseMetaHandler(const string &database_meta_path)
{
    // TODO：path检查（路径是否合法、是否真的有文件）
    // 如果不合法直接抛出异常，因为此处路径不由用户决定，如果出现异常一定是代码的问题
    // 如果是不存在就创建一个
    database_meta_path_ = database_meta_path;
    load();
}

const string &DatabaseMetaHandler::get_database_meta_path()
{
    return database_meta_path_;
}

const json &DatabaseMetaHandler::get_database_meta()
{
    return database_meta_;
}

// 使用传入的路径保存meta，即“另存为”
void DatabaseMetaHandler::save(const string &database_meta_path)
{
    // TODO：检查数据库名、meta信息、路径是否非空

    status_code error;
    DataFileHandler file;

    error = file.open(database_meta_path, 1, 0, 1, 0);
    if (error)
        raise_exception(error);

    error = file.append(database_meta_.dump());
    if (error)
        raise_exception(error);
}

// 使用内置的成员变量存储（覆盖原文件数据）
void DatabaseMetaHandler::save()
{
    // TODO：检查数据库名、meta信息、路径是否非空

    status_code error;
    DataFileHandler file;

    error = file.open(database_meta_path_, 1, 0, 1, 0);
    if (error)
        raise_exception(error);

    error = file.append(database_meta_.dump());
    if (error)
        raise_exception(error);
}

void DatabaseMetaHandler::load()
{
    DataFileHandler file;

    // 构造函数中的路径检查会保证这个表路径是合法的
    status_code error = file.open(database_meta_path_, 0, 0, 0, 1);

    if (error)
        raise_exception(error);

    respond<string> json_string = file.read_all();

    // 返回非空字符串则状态为SUCCESS
    if (!json_string.first.empty())
    {
        // TODO：应该多一步JSON合法性检查
        database_meta_ = json::parse(json_string);
    }
    else
    {
        raise_exception(json_string.second);
    }
}

status_code DatabaseMetaHandler::add_database_in_json(const string &database_name)
{
    // 检查数据库是否已经存在
    auto it = database_meta_.find(database_name);

    // 不存在
    if (it == database_meta_.end())
    {
        database_meta_[database_name] = json::object();
        return error_code::SUCCESS;
    }
    else // 存在
    {
        return error_code::ERROR_DATABASE_HAS_ALREADY_EXISTS;
    }
}

status_code DatabaseMetaHandler::add_table_in_json(const string &database_name, const string &table_name, const string &table_dir)
{
    // 检查表是否已经存在
    auto it = database_meta_.find(database_name);

    // 不存在
    if (it == database_meta_.end())
    {
        return error_code::ERROR_TABLE_HAS_ALREADY_EXISTS;
    }
    else // 存在
    {
        database_meta_[database_name][table_name] = table_dir;
        return error_code::SUCCESS;
    }
}

status_code DatabaseMetaHandler::delete_database_in_json(const string &database_name)
{
    auto it = database_meta_.find(database_name);
    if (it == database_meta_.end())
    {
        return error_code::ERROR_DATABASE_NOT_EXISTS;
    }
    else
    {
        database_meta_.erase(it);
        return error_code::SUCCESS;
    }
}

status_code DatabaseMetaHandler::delete_table_in_json(const string &database_name, const string &table_name)
{
    auto it = database_meta_[database_name].find(table_name);
    if (it == database_meta_[database_name].end())
    {
        return error_code::ERROR_TABLE_NOT_EXISTS;
    }
    else
    {
        database_meta_.erase(it);
        return error_code::SUCCESS;
    }
}