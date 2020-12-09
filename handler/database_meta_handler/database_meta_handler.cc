#include "database_meta_handler.h"

DatabaseMetaHandler::DatabaseMetaHandler(const string &database_meta_dir, json &database_meta)
{
    // data文件夹不存在，则创建
    if (!access(database_meta_dir.c_str(), 0) == 0)
    {
        mkdir(database_meta_dir.c_str(), S_IRWXU);
    }

    database_meta_path_ = database_meta_dir + "/db_meta.json";
    database_meta_ = database_meta;
}

DatabaseMetaHandler::DatabaseMetaHandler(const string &database_meta_dir)
{
    // 文件夹不存在，则创建
    if (!access(database_meta_dir.c_str(), 0) == 0)
    {
        mkdir(database_meta_dir.c_str(), S_IRWXU);
    }

    database_meta_path_ = database_meta_dir + "/db_meta.json";
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
status_code DatabaseMetaHandler::save(const string &database_meta_path)
{
    // TODO：检查数据库名、meta信息、路径是否非空

    DataFileHandler file;

    file.open(database_meta_path, 1, 0, 1, 0);
    return file.append(database_meta_.dump());
}

// 使用内置的成员变量存储（覆盖原文件数据）
status_code DatabaseMetaHandler::save()
{
    // TODO：检查数据库名、meta信息、路径是否非空
    DataFileHandler file;

    file.open(database_meta_path_, 1, 0, 1, 0);
    return file.append(database_meta_.dump());
}

status_code DatabaseMetaHandler::load()
{
    DataFileHandler file;

    // 数据库头文件不存在则创建
    if (!file.file_exist(database_meta_path_))
    {
        // 创建文件
        status_code err = file.open(database_meta_path_);

        if (err)
            return err;

        // 写入空JSON
        file.append("{}");

        // 空JSON赋值
        database_meta_ = json::object();

        return error_code::SUCCESS;
    }

    // 以读取的模式打开
    status_code err = file.open(database_meta_path_, 0, 0, 0, 1);

    if (err)
        return err;

    respond<string> json_string = file.read_all();
    err = json_string.second;

    if (err)
    {
        return err;
    }
    else
    {
        database_meta_ = json::parse(json_string.first);
        return error_code::SUCCESS;
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
        database_meta_[database_name].erase(it);
        return error_code::SUCCESS;
    }
}