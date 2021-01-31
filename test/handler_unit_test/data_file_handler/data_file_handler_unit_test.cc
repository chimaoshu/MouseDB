#include <iostream>
#include "src/data_file_handler/data_file_handler.h"
#include <string>

using namespace std;

class DataFileHandlerUnitTest
{
public:
    void test_open()
    {
        DataFileHandler a;
        a.open("./test/handler_unit_test/data_file_handler/test.json", 0, 0, 0, 1);
        respond<string> b = a.read_all_text();
        cout << b.first;
    }

    void test_append()
    {
        DataFileHandler a, b;
        a.open("./test/handler_unit_test/data_file_handler/test.json", 1, 1, 0, 0);
        a.append("\ntest");
    }

    void test_read_all()
    {
        DataFileHandler a;
        a.open("./test/handler_unit_test/data_file_handler/test.json", 0, 0, 0, 1);
        respond<string> b = a.read_all_text();
        cout << b.first << endl;
    }

    void test_create_file()
    {
        DataFileHandler a;
        a.open("./test/handler_unit_test/data_file_handler/test2.json", 1, 0, 1, 0);
    }

    void test_read_lines_into_buffer()
    {
        DataFileHandler a;
        a.open("./test/handler_unit_test/data_file_handler/test.json", 0, 0, 0, 1);
        auto res = a.read_lines_into_buffer(0, 2, 5);
        void *p = res.first;
        int available_lines = res.second;
        cout << available_lines << endl;
        cout << (char*)p << endl;
    }
};

int main()
{
    // cout << 123 << endl;
    DataFileHandlerUnitTest a;
    a.test_append();
    a.test_open();
    a.test_read_all();    
    a.test_create_file();
    a.test_read_lines_into_buffer();
    return 0;
}
