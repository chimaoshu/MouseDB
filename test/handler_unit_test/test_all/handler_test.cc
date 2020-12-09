#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#include "handler/action_processor/action_processor.h"

using namespace std;

using json = nlohmann::json;

void run()
{
    UserAction ua = UserAction();

    ua.create_a_database("foo");
    ua.show_database_names();

    // 打开数据库
    ua.use_a_database("foo");

    string path = get_current_dir_name();
    path += "/test/handler_unit_test/test_all/";

    ifstream i1(path + "meta.json");
    json meta;
    i1 >> meta;
    ua.create_a_table("student", meta);
    i1.close();

    ua.print_debug_info();

    // 读取JSON

    ifstream i(path + "test_data_100.json");
    json j_100;
    i >> j_100;
    i.close();

    // ifstream i(path + "test_data_100.json");
    // json j_1000;
    // i >> j_1000;
    // i.close();

    // ifstream i(path + "test_data_100.json");

    // json j_10000;
    // i >> j_10000;
    // i.close();

    // ifstream i(path + "test_data_100.json");
    // json j_100000;
    // i >> j_100000;
    // i.close();

    // rows
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);
    // ua.append("student", j_100);

    list<string> wanted_column;
    wanted_column.push_back("foo");
    wanted_column.push_back("bar");
    wanted_column.push_back("barz");

    string table_name = "student";
    cout << "---------" << endl;
    ua.get_lines(42, 30, table_name, wanted_column, true);
    cout << "---------" << endl;
    ua.get_lines(90, 20, table_name, wanted_column, true);
    cout << "---------" << endl;
    ua.get_lines(42, 30, table_name, wanted_column, true);
    cout << "---------" << endl;
    ua.get_lines(42, 30, table_name, wanted_column, true);
    cout << "---------" << endl;

    ua.drop_a_table(table_name);
    ua.drop_a_database("foo");
}

int main()
{
    run();
    return 0;
}