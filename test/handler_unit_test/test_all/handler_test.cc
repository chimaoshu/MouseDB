/*
自动生成的代码：
*/
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
/*
在此处include你需要用到的libs
*/

#include "handler/action_processor/action_processor.h"

using namespace std;

// 如果你和我一样用的是nlohmann/json，那么使用下面的句子，否则注释掉他
using json = nlohmann::json;

void run()
{
    UserAction ua = UserAction();

    /*
    以你自己的设计思路，在接下来的几行中创建一个数据库：
    */

    ua.create_a_database("foo");
    // ua.print_debug_info();
    ua.show_database_names();

    // 打开数据库
    ua.use_a_database("foo");

    // 也许你会对相对路径感到混乱，那么请在这里填上四个json所在的文件夹
    string path = get_current_dir_name();
    path += "/test/handler_unit_test/test_all/";

    ifstream i1(path + "meta.json");
    json meta;
    i1 >> meta;
    ua.create_a_table("student", meta);
    i1.close();

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
    // j_100.~basic_json();
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

    ua.query(42,30,"student", wanted_column,true);
    cout << "---------" << endl;
    ua.query(90,20,"student", wanted_column,true);

    // //也许你应该停一停
    // system("pause");

    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);
    // row2.write(j_1000);

    // cout << (&row1.read(4, 10)) << endl;
    // cout << (&row1.read(100, 56)) << endl;

    // //也许你应该停一停
    // system("pause");

    // row3.write(j_10000);
    // row3.write(j_10000);
    // row3.write(j_10000);
    // row3.write(j_10000);
    // row3.write(j_10000);

    // cout << (&row1.read(4, 10)) << endl;
    // cout << (&row1.read(100, 56)) << endl;

    // //也许你应该停一停
    // system("pause");

    // row4.write(j_100000);

    // cout << (&row1.read(4, 10)) << endl;
    // cout << (&row1.read(100, 56)) << endl;

    // system("pause");
    ua.drop_a_table("student");
    ua.drop_a_database("foo");
}

int main()
{
    run();
    return 0;
}
