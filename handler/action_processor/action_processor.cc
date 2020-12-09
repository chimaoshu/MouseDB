#include "action_processor.h"

UserAction::UserAction()
    : current_dir_name_(get_current_dir_name()),
      db_meta_dir_(current_dir_name_ + "/data"),
      db_meta_handler_(db_meta_dir_),
      db_handler_(db_meta_handler_, db_meta_dir_)
{
}

UserAction::UserAction(string work_folder)
    : current_dir_name_(work_folder),
      db_meta_dir_(current_dir_name_ + "/data"),
      db_meta_handler_(db_meta_dir_),
      db_handler_(db_meta_handler_, db_meta_dir_)
{
}

void UserAction::print_debug_info()
{
    cout << "\n=========================debug_info============================\n";
    cout << "current_dir_name: " << current_dir_name_ << '\n';
    cout << "db_meta_dir: " << db_meta_dir_ << '\n';
    cout << "\n===============database_meta==================\n";
    cout << db_meta_handler_.get_database_meta() << '\n';
    cout << "\n===============database_meta==================\n";

    if (current_used_database_)
    {
        cout << "current used database name" << current_used_database_->get_database_name() << "table info:";
        auto table_names = current_used_database_->get_all_table_names();
        for (auto it = table_names.begin(); it != table_names.end(); it++)
        {
            cout << "table name:" << *it << '\n';
            cout << "table dir:" << current_used_database_->get_table_dir(*it) << '\n';
            cout << "table meta:" << current_used_database_->get_table_meta(*it) << '\n';
        }
        cout << endl;
    }

    cout << "\n=========================debug_info============================\n";
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
        cout << "database names:" << *it << ' ';
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
            map_of_table_name_to_table_handler_[database_name] = table_handler;
            current_used_database_ = map_of_table_name_to_table_handler_[database_name];
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
    string database_path = current_used_database_->get_table_dir(table_name) + '/' + table_name + ".mdb";

    // 打开table_row_handler
    TableRowHandler table_row_handler(
        database_path,
        current_used_database_->get_table_meta_handler(table_name));

    // 写入数据
    table_row_handler.write(rows_info);
}

void UserAction::get_lines(
    uint32_t off_set,
    uint32_t line_number,
    const string &table_name,
    list<string> &wanted_column_names,
    bool debug_mode)
{
    // 实例化 TableRowHandler
    TableRowHandler table_row_handler(
        current_used_database_->get_table_dir(table_name) + '/' + table_name + ".mdb",
        current_used_database_->get_table_meta_handler(table_name));

    // 生成要求的列的编号
    list<int> wanted_column_orders = current_used_database_
                                         ->get_table_meta_handler(table_name)
                                         ->get_column_orders_by_names(wanted_column_names);

    // debug
    if (debug_mode)
    {
        // 把数据读取后序列化为JSON
        json *outcome = table_row_handler.read<json>(off_set, line_number, wanted_column_orders);
        cout << *outcome << endl;

        delete outcome;
        return;
    }

    // 把数据读取后序列化为list
    list<json> *outcome = table_row_handler.read<list<json>>(off_set, line_number, wanted_column_orders);
    cout << *outcome << endl;

    // 后续其他操作
    delete outcome;
}