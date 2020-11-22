#include <iostream>
#include "data_file_handler.h"
#include <string>

using namespace std;

class DataFileHandlerUnitTest
{
public:
    void test_open()
    {
        DataFileHandler a;
        a.open("./test/test.json", 0, 0, 0, 1);
        respond<string> b = a.read_all();
        cout << b.first;
    }

    void test_append()
    {
        DataFileHandler a;
        // 必须是存在的文件路径
        a.open("test/test.json", 1, 1, 0, 0);
        a.append("\ntest");
    }

    void test_read_line()
    {
        DataFileHandler a;
        a.open("./test/test.json", 0, 0, 0, 1);
        respond<string> b = a.read_line();
        cout << b.first << endl;
    }

    void test_read_all()
    {
        DataFileHandler a;
        a.open("./test/test.json", 0, 0, 0, 1);
        respond<string> b = a.read_all();
        cout << b.first << endl;
    }

    void test_create_file()
    {
        DataFileHandler a;
        a.open("./test/test2.json", 1, 0, 1, 0);
    }

    void test_read_lines_into_buffer()
    {
        DataFileHandler a;
        a.open("./test/test2.json", 0, 0, 0, 1);
        void *p = a.read_lines_into_buffer(0, 2, 5);
        cout << (char*)p << endl;
    }
};

int main()
{
    // cout << 123 << endl;
    DataFileHandlerUnitTest a;
    // a.test_append();
    // a.test_open();
    // a.test_read_all();
    // a.test_read_line();
    // a.test_create_file();
    a.test_read_lines_into_buffer();
    return 0;
}
