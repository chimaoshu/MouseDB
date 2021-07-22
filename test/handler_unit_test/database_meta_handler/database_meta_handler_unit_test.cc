#include <iostream>

#include "src/database_meta_handler/database_meta_handler.h"

#include "third_part/Json/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class DatabaseMetaHandlerUnitTest {
public:
  void test_load_and_save() {
    DatabaseMetaHandler *table = new DatabaseMetaHandler(
        "./test/handler_unit_test/database_meta_handler/test.json");
    json j = table->get_database_meta();

    table->save(
        "./test/handler_unit_test/database_meta_handler/test_save.json");

    return;
  }
};

int main() {
  DatabaseMetaHandlerUnitTest a;
  a.test_load_and_save();
  return 0;
}
