#include <iostream>

#include "database_meta_handler.h"

#include "lib/Json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class DatabaseMetaHandlerUnitTest
{
public:
    void test_load_and_save()
    {
        // load
        DatabaseMetaHandler *table = new DatabaseMetaHandler("./test/test.json");
        json j = table->get_database_meta();

        j["foo2"] = "bar2"; 
        string a = j.dump();

        cout << a << endl;
        table->set_database_meta_information(j);

        // save
        table->save("./test/test2.json"); // 如果没有前面的小圆点就会打开文件错误
        table->save(); 

        return;
    }
};

int main()
{
    DatabaseMetaHandlerUnitTest a;
    a.test_load_and_save();
    return 0;
}
