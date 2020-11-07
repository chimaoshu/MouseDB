#include <iostream>

#include "table_meta_handler.h"

#include "storage/Json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class TableMetaHandlerUnitTest
{
public:
    void test_load_and_save()
    {
        // load
        TableMeta *table = new TableMeta("./test/test.json", "table1");
        json j = table->get_keys_information();

        j["foo"] = "bar"; 
        string a = j.dump();

        cout << a << endl;
        table->set_keys_information(j);

        // save
        table->save("/test/test2.json");
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
