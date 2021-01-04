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
        map_of_table_name_to_table_meta_handler_[table_name] = new TableMetaHandler(table_meta_path);
    }
}

TablesHandler::~TablesHandler()
{
    // 遍历存储在map中的TableMetaHandler
    for (auto it = map_of_table_name_to_table_meta_handler_.begin(); it != map_of_table_name_to_table_meta_handler_.end(); it++)
    {
        // 释放TeblaMetaHandler
        if (it->second)
            delete it->second;
    }

    // 遍历存储在map中的HotDataManager
    for (auto it = map_of_table_name_to_hot_data_manager_.begin(); it != map_of_table_name_to_hot_data_manager_.end(); it++)
    {
        // 释放HotDataManager
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

status_code TablesHandler::create_table(const string &table_directory, const string &table_name, json &table_meta, DatabaseMetaHandler &db_mete_handler)
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
    map_of_table_name_to_table_meta_handler_[table_name] = new TableMetaHandler(table_meta_path, table_meta);

    // 保存表头JSON到磁盘，并返回状态码
    return map_of_table_name_to_table_meta_handler_[table_name]->save();
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
    auto it = map_of_table_name_to_table_meta_handler_.find(table_name);
    if (it != map_of_table_name_to_table_meta_handler_.end())
        map_of_table_name_to_table_meta_handler_.erase(it);

    // 返回状态码
    return error;
}

list<string> TablesHandler::get_all_table_names()
{
    list<string> all_table_name;

    // 遍历map_of_name_and_table_meta_handler的keys
    for (auto it = map_of_table_name_to_table_meta_handler_.begin(); it != map_of_table_name_to_table_meta_handler_.end(); it++)
    {
        all_table_name.push_back(it->first);
    }
    return all_table_name;
}

const json &TablesHandler::get_table_meta(const string &table_name)
{
    // 取得表名对应的TableMetaHandler实例
    auto it = map_of_table_name_to_table_meta_handler_.find(table_name);

    if (it != map_of_table_name_to_table_meta_handler_.end())
        // 通过TableMetaHandler实例获得表头信息
        return it->second->get_table_meta();
    else
        return NULL;
}

 HotDataManager *&TablesHandler::get_hot_data_manager(const std::string &table_name)
{
    auto it = map_of_table_name_to_hot_data_manager_.find(table_name);

    // 若存在则返回
    if (it != map_of_table_name_to_hot_data_manager_.end())
    {
        return it->second;
    }
    // 不存在则创建
    else
    {
        string hot_data_file_path = this->get_table_dir(table_name) + '/' + table_name + ".hot";

        // 打开table_row_handler
        map_of_table_name_to_hot_data_manager_[table_name] = new HotDataManager(
            hot_data_file_path,
            this->get_table_meta_handler(table_name));

        return map_of_table_name_to_hot_data_manager_[table_name];
    }
}

 ColdDataManager *&TablesHandler::get_cold_data_manager(const std::string &table_name)
{
    auto it = map_of_table_name_to_cold_data_manager_.find(table_name);

    // 若存在则返回
    if (it != map_of_table_name_to_cold_data_manager_.end())
    {
        return it->second;
    }
    // 不存在则创建
    else
    {
        string cold_data_file_path = this->get_table_dir(table_name) + '/' + table_name + ".hot";

        // 打开table_row_handler
        // 传入TableMeta让其自己从中取出正在使用的数据文件
        map_of_table_name_to_cold_data_manager_[table_name] = new ColdDataManager(
            cold_data_file_path,
            this->get_table_meta_handler(table_name));

        return map_of_table_name_to_cold_data_manager_[table_name];
    }
}

 void TablesHandler::set_hot_data_manager_in_map(const string &table_name, HotDataManager *new_hot_data_manager)
{
    map_of_table_name_to_hot_data_manager_[table_name] = new_hot_data_manager;
}

 void TablesHandler::set_cold_data_manager_in_map(const string &table_name, ColdDataManager *new_cold_data_manager)
{
    map_of_table_name_to_cold_data_manager_[table_name] = new_cold_data_manager;
}

status_code TablesHandler::dump_table_in_sub_thread(const string &table_name)
{
    // 运行在子线程中

    // 获取旧的HotDataManager
    HotDataManager *old_hot_data_manager = get_hot_data_manager(table_name);

    // 进行hot-dump操作，生成新的热数据文件、冷数据文件，返回新的HotDataManager
    // 此时TableMata中已经存储了新的冷数据文件的文件名了
    HotDataManager *new_hot_data_manager = old_hot_data_manager->dump_to_cold_data();

    // 获取现在table对应的TableMetaHandler
    TableMetaHandler *current_table_meta_handler = get_table_meta_handler(table_name);

    // 进入切换模式

    // 1、把HotDataManager的is_switching改为true
    // 此时暂停查询操作，把查询请求缓存到消息队列中
    old_hot_data_manager->change_to_switch_mode();

    // 2、更改TableMeta指向的现用数据库的内容
    // 并把转正的冷数据添加到历史版本，并保存到磁盘
    current_table_meta_handler->change_new_hot_data_file_to_current_used();
    current_table_meta_handler->change_new_cold_data_file_to_current_used();
    current_table_meta_handler->save();

    // 3、生成新的ColdDataManager
    // 修改表名对应的ColdDataManger为缓存的ColdDataManager，并删除旧的manager

    // 冷数据文件名从传入的TableMeta中取出（即新的冷数据文件）
    ColdDataManager *new_cold_data_manager = new ColdDataManager(
        this->get_table_dir(table_name),
        current_table_meta_handler
    );

    delete this->get_cold_data_manager(table_name);
    this->set_cold_data_manager_in_map(table_name, new_cold_data_manager);
    

    // 4、修改表名对应的HotDataManger为缓存的HotDataManager，并删除旧的manager
    // 此时查询操作来的时候，检查到新HotDataManager的is_switching为false
    // 此时又可以进行查询操作了，并且使用新一套的HotDataManger和ColdDataManger进行查询了
    delete old_hot_data_manager;
    this->set_hot_data_manager_in_map(table_name, new_hot_data_manager);

    return error_code::SUCCESS;
}

status_code TablesHandler::dump_table(const string &table_name)
{
    thread hot_dump_thread(&TablesHandler::dump_table_in_sub_thread, this, table_name);

    // 与主线程分离进行
    hot_dump_thread.detach();
}