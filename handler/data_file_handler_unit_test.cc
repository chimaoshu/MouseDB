#include <iostream>
#include "handler/data_file_handler.h"
#include <string>

using namespace std;

class DataFileHandlerUnitTest
{
public:
    void test_open()
    {
        DataFileHandler a;
        a.open("./test/test.json", 0, 0, 0, 1);
        string b = a.read_all();
        cout << b;
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
        string b = a.read_line();
        cout << b << endl;
    }

    void test_read_all()
    {
        DataFileHandler a;
        a.open("./test/test.json", 0, 0, 0, 1);
        string b = a.read_all();
        cout << b << endl;
    }
};

int main()
{
    cout << 123 << endl;
    DataFileHandlerUnitTest a;
    a.test_append();
    // a.test_open();
    // a.test_read_all();
    // a.test_read_line();
    return 0;
}
