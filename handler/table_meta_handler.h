#ifndef MOUSEDB_HANDLER_TABLE_META_HANDLER_H_
#define MOUSEDB_HANDLER_TABLE_META_HANDLER_H_

#include <string>
#include <fstream>

#include "storage/Json/single_include/nlohmann/json.hpp"

using namespace std;

// for convenience
using json = nlohmann::json;

class TableMeta
{
private:
    string table_name_;
    string file_storage_path_;

    // 存储keys信息的JsonArray，格式见DevDairy/头文件格式.jsonc
    json keys_information_;

public:
    // DataFileHandler file_;

    // read only
    const string &get_table_name();
    const string &get_file_storage_path();
    const json &get_keys_information();

    void set_table_name(const string &table_name);
    void set_file_storage_path(const string &file_storage_path);
    void set_keys_information(const json &keys_information);

    TableMeta(const string &file_storage_path, const string &table_name); // 通过load创建
    TableMeta(const string &file_storage_path, const string &table_name, json &keys_information);
    
    void save(); // 使用类成员变量file_storage_path_
    void save(const string&path); // 使用传入的路径

    void load();
};
#endif // MOUSEDB_HANDLER_TABLE_META_HANDLER_H_