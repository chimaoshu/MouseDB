#include "action_processor.h"

UserAction::UserAction()
    : db_meta_handler_(db_meta_dir_),
      db_handler_(db_meta_handler_, db_meta_dir_)
{
}

void UserAction::show_database_names()
{
    list<string> database_names = db_handler_.get_all_database_name();

    if (database_names.empty())
    {
        cout << "no database" << '\n';
        return;
    }

    for (auto it = database_names.begin(); it != database_names.end(); it++)
    {
        cout << *it << ' ';
    }
    cout << '\n';
}

void UserAction::create_a_database(const string &database_name)
{
    status_code err = db_handler_.create(database_name);
    if (err)
        cout << "database create error:" << err << '\n';
}

void UserAction::drop_a_database(const string &database_name)
{
    status_code err = db_handler_.drop(database_name);
    if (err)
        cout << "database drop error:" << err << '\n';
}

void UserAction::use_a_database(const string &database_name)
{
    auto it = map_of_table_name_to_table_handler_.find(database_name);

    // 若数据库已经在缓存中
    if (it != map_of_table_name_to_table_handler_.end())
    {
        current_used_database_ = map_of_table_name_to_table_handler_[database_name];
    }
    else
    {
        TableHandler *table_handler = db_handler_.open(database_name);
        if (!table_handler)
        {
            cout << "error: fail to open the database, database not exists.\n";
        }
        else
        {
            current_used_database_ = map_of_table_name_to_table_handler_[database_name];
            map_of_table_name_to_table_handler_[database_name] = table_handler;
        }
    }
}

void UserAction::show_table_names()
{
    if (!current_used_database_)
        raise_exception(error_code::ERROR_TABLE_NOT_OPENED);

    list<string> table_names = current_used_database_->get_all_table_names();

    if (table_names.empty())
    {
        cout << "no table" << '\n';
        return;
    }

    for (auto it = table_names.begin(); it != table_names.end(); it++)
    {
        cout << *it << ' ';
    }
    cout << '\n';
}

void UserAction::create_a_table(const string &table_name, json &table_meta)
{
    if (!current_used_database_)
        raise_exception(error_code::ERROR_TABLE_NOT_OPENED);

    //  data/数据库名/表名/
    string default_table_dir = db_meta_dir_ + '/' + current_used_database_->get_database_name() + '/' + table_name;

    // 创建文件夹
    mkdir(default_table_dir.c_str(), S_IRWXU);

    // 建表
    status_code err = current_used_database_->create(default_table_dir, table_name, table_meta, db_meta_handler_);

    if (err)
        cout << "table create error:" << err << '\n';
}

void UserAction::drop_a_table(const string &table_name)
{
    status_code err = current_used_database_->drop_table(table_name, db_meta_handler_);

    if (err)
        cout << "table drop error:" << err << '\n';
}

void UserAction::append(const string &table_name, json &rows_info)
{
    // 打开table_row_handler
    TableRowHandler table_row_handler(
        current_used_database_->get_table_dir(table_name) + '/' + table_name + ".mdb",
        current_used_database_->get_table_meta_handler(table_name));

    // 写入数据
    table_row_handler.write(rows_info);
}

void UserAction::query(
    uint64_t off_set,
    uint64_t line_number,
    const string table_name,
    list<string> &wanted_column_names,
    bool debug_mode)
{
    TableRowHandler table_row_handler(
        current_used_database_->get_table_dir(table_name) + '/' + table_name + ".mdb",
        current_used_database_->get_table_meta_handler(table_name));

    list<int> wanted_column_orders = current_used_database_
                                         ->get_table_meta_handler(table_name)
                                         ->get_column_orders_by_names(wanted_column_names);

    if (debug_mode)
    {
        vector<json> *outcome = table_row_handler.read<vector<json>>(off_set, line_number, wanted_column_orders);
        cout << *outcome << endl;
        delete outcome;
    }
    else
    {
        // 后续其他操作
        json *outcome = table_row_handler.read<json>(off_set, line_number, wanted_column_orders);
        cout << *outcome << endl;
        delete outcome;
    }
}