#ifndef MOUSEDB_HANDLER_ACTION_PROCESSOR_ACTION_PROCESSOR_H_
#define MOUSEDB_HANDLER_ACTION_PROCESSOR_ACTION_PROCESSOR_H_

#include <sys/stat.h>
#include<stdint.h>

#include <iostream>
#include "lib/Json/single_include/nlohmann/json.hpp"

#include "handler/exception_handler/exception_handler.h"
#include "handler/data_file_handler/data_file_handler.h"
#include "handler/table_meta_handler/table_meta_handler.h"
#include "handler/table_handler/table_handler.h"
#include "handler/table_row_handler/table_row_handler.h"
#include "handler/database_handler/database_handler.h"
#include "handler/database_meta_handler/database_meta_handler.h"


class UserAction
{
private:
    // linux下获取当前文件目录
    string current_dir_name_ = get_current_dir_name();

    // 数据所在目录
    string db_meta_dir_ = current_dir_name_ + "/data";

    // 当前打开的数据库
    TableHandler *current_used_database_ = NULL;

    DatabaseMetaHandler db_meta_handler_;

    DatabaseHandler db_handler_;

    // 存储从数据库名到数据库的映射（一个数据库就是一个table_handler）
    // 每当有数据库被打开，那么这里就会多存储一个数据库的信息
    map<string, TableHandler *> map_of_table_name_to_table_handler_;

public:
    // 当用户打开了程序，即读取数据库信息
    UserAction();

    // 显示调试信息
    void print_debug_info();

    // 显示所有数据库名
    void show_database_names();

    // 创建一个数据库
    void create_a_database(const string &database_name);

    // 删除一个数据库
    void drop_a_database(const string &database_name);

    // 打开一个数据库
    void use_a_database(const string &database_name);

    // 显示所有表
    void show_table_names();

    // 创建一个表
    void create_a_table(const string &table_name, json &table_meta);

    // 删除一个表
    void drop_a_table(const string &table_name);

    // 追加几行（目前缺少对rows_info的正确性检验）
    // 后续应该支持protobuf而非JSON数据的传入
    void append(const string &table_name, json &rows_info);

    // 查询几行，如果是调试模式返回json，否则返回list
    void query(
        uint32_t off_set,
        uint32_t line_number,
        const string table_name,
        list<string> &wanted_column_names,
        bool debug_mode);
};
#endif