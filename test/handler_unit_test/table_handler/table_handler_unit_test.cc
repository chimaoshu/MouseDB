#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#include "src/action_processor/action_processor.h"

using namespace std;

using json = nlohmann::json;

class TablesHandlerUnitTest
{
private:
    string test_dir = string(get_current_dir_name()) + "/test/handler_unit_test/table_handler";
    int nothing; // 移除data文件夹
    UserAction ua;

public:
    // 打开数据库
    TablesHandlerUnitTest()
        : nothing(remove(test_dir.c_str())),
          ua(UserAction(test_dir))
    {
        ua.create_a_database("foo");
        ua.show_database_names();
        ua.use_a_database("foo");
    }

    void test_create()
    {
        string path = get_current_dir_name();
        path += "/test/handler_unit_test/test_all/";

        ifstream i1(path + "meta.json");
        json meta;
        i1 >> meta;
        i1.close();

        // 即TablesHandler的create操作
        ua.create_a_table("student", meta);
    }

    void test_drop_table()
    {
        ua.drop_a_table("student");
    }

    void test_get()
    {
        ifstream i(test_dir + "/data/db_meta.json");
        json db_meta;
        i >> db_meta;

        TablesHandler th("foo", db_meta);
        auto list = th.get_all_table_names();
        for (auto it = list.begin(); it != list.end(); it++)
        {
            cout << *it << ' ' << '\n';
        }
        cout << '\n';

        string table_name = "student";
        cout << th.get_database_name() << endl;
        cout << th.get_table_dir(table_name) << endl;

        // 即th.get_table_meta()，由于th是另外开的，因此create的时候没有缓存信息，所以没办法直接用
        ua.print_debug_info();
    }

    ~TablesHandlerUnitTest()
    {
        ua.drop_a_database("foo");
    }
};

int main()
{
    TablesHandlerUnitTest a;
    a.test_create();
    a.test_get();
    a.test_drop_table();
}