#include "table_handler.h"

class TableHandlerUnitTest
{
public:
    // 若函数正常运行
    // 将会生成一个test/一班学生表.json
    // 与test/binary_path.db
    void test_create_and_drop()
    {
        list<column *> l;

        struct column *c1 = new column;
        c1->key_name = "学号";
        c1->type = "int";
        c1->is_primary = 1;
        c1->not_null = 1;

        struct column *c2 = new column;
        c2->key_name = "姓名",
        c2->type = "char[3]";
        c2->is_primary = 0;
        c2->not_null = 0;

        struct column *c3 = new column;
        c3->key_name = "分数",
        c3->type = "int";
        c3->is_primary = 0,
        c3->not_null = 0;

        l.push_back(c1);
        l.push_back(c2);
        l.push_back(c3);

        TableHandler th;
        th.create(string("test/"), string("test/binary_path.db"), string("一班学生表"), l);

        // 自己打个断点看创建了没
        th.drop_table();
    }

    void test_drop_1()
    {
        list<column *> l;

        struct column *c1 = new column;
        c1->key_name = "学号";
        c1->type = "int";
        c1->is_primary = 1;
        c1->not_null = 1;

        struct column *c2 = new column;
        c2->key_name = "姓名",
        c2->type = "char[3]";
        c2->is_primary = 0;
        c2->not_null = 0;

        struct column *c3 = new column;
        c3->key_name = "分数",
        c3->type = "int";
        c3->is_primary = 0,
        c3->not_null = 0;

        l.push_back(c1);
        l.push_back(c2);
        l.push_back(c3);

        TableHandler th;
        th.create("./test/", "test/一班学生数据库.db", "一班学生表", l);

        TableHandler th2;
        th2.drop_table("./test/一班学生表.json");
    }
};

int main()
{
    TableHandlerUnitTest a;
    a.test_create_and_drop();
    a.test_drop_1();
    return 0;
}