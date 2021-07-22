#include <iostream>

#include "src/table_meta_handler/table_meta_handler.h"

#include "third_part/Json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class TableMetaHandlerUnitTest {
public:
  void test_load_and_save() {
    // load
    TableMetaHandler *table = new TableMetaHandler(
        "./test/handler_unit_test/table_meta_handler/test.json");
    json j = table->get_table_meta();

    cout << j << endl;

    // save
    table->save("./test/handler_unit_test/table_meta_handler/"
                "table_meta_handler_save_test.json");

    return;
  }
};

int main() {
  TableMetaHandlerUnitTest a;
  a.test_load_and_save();
  return 0;
}
