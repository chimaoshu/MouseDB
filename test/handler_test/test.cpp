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
    ua.show_database_names();

    /*
    以你自己的设计思路，在接下来的几行中创建一个属于上述数据库的表：
    表包含三列的信息：
    第一列表示学号：int order
    第二列表示班级：int class
    第三列表示分数：int score
    */

    ifstream i("meta.json");
    json meta;
    i >> meta;
    ua.create_a_table("student", meta);

    /*
    以你自己的设计思路，在接下来的几行中创建一个TableRow的四个实例row1，row2，row3，row4：
    */


    // 读取JSON

    // 也许你会对相对路径感到混乱，那么请在这里填上四个json所在的文件夹
    string path = "/home/chenjingwei/workspace/MouseDB/test/test_all/";

    ifstream i(path + "test_data_100.json");
    json j_100;
    i >> j_100;
    i.close();

    ifstream i(path + "test_data_100.json");
    json j_1000;
    i >> j_1000;
    i.close();

    ifstream i(path + "test_data_100.json");
    json j_10000;
    i >> j_10000;
    i.close();

    ifstream i(path + "test_data_100.json");
    json j_100000;
    i >> j_100000;
    i.close();

    // rows
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

    // ua.query(42,4323,"student", wanted_column,true);
    ua.query(42,10,"student", wanted_column,true);

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
}

int main()
{
    run();
    return 0;
}
