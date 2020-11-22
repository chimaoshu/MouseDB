#include <iostream>

#include "table_meta_handler.h"

#include "lib/Json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class TableMetaHandlerUnitTest
{
public:
    void test_load_and_save()
    {
        // load
        TableMetaHandler *table = new TableMetaHandler("./test/test.json");
        json j = table->get_table_meta();

        j["foo"] = "bar"; 
        string a = j.dump();

        cout << a << endl;
        table->set_table_meta_information(j);

        // save
        table->save("./test/test2.json"); // 如果没有前面的小圆点就会打开文件错误
        table->save(); 

        return;
    }
};

int main()
{
    TableMetaHandlerUnitTest a;
    a.test_load_and_save();
    return 0;
}
