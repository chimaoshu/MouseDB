#include <iostream>
#include <string>

#include "src/data_file_handler/data_file_handler.h"
#include "src/utils/tools.h"

using namespace std;

class DataFileHandlerUnitTest {

private:
  const string file_path =
      "./test/handler_unit_test/data_file_handler/test.json";

public:
  // 测试创建文件的功能
  void test_create() {

    Tools::remove_file(file_path.c_str());

    DataFileHandler a;
    a.open(file_path.c_str(), 1, 0, 0, 0);

    assert(a.is_open());
  }

  // 测试写与读的功能
  void test_read_and_write() {

    Tools::remove_file(file_path.c_str());

    // 创建文件
    DataFileHandler a;
    a.open(file_path.c_str(), 1, 0, 0, 0);
    a.close();
    assert(DataFileHandler::file_exist(file_path.c_str()));

    // 写
    a.open(file_path.c_str(), 1, 1, 0, 1);
    a.append(file_path);

    // 读
    auto res = a.read_all_text();
    assert(res.second == StatusCode::SUCCESS && res.first == file_path);

    // 二进制读
    auto res2 = a.read_lines_into_buffer(0, 1, 1, false, true);
    int str_len = res2.second;
    char *str = (char *)res2.first;

    // 逐个字符比较
    for (int i = 0; i < str_len; i++) {
      char tmp = *(str + i);
      assert(tmp == file_path[i]);
    }

    // 二进制读2
    for (int i = 0; i < str_len; i++) {
      a.move_g_cursor(0);
      char *str2 = (char *)a.read_primary_key_into_buffer(0, 1, true);
      assert(*str2 == file_path[i]);
    }
  }

  ~DataFileHandlerUnitTest() { Tools::remove_file(file_path.c_str()); }
};

int main() {
  DataFileHandlerUnitTest a;
  a.test_create();
  a.test_read_and_write();
  cout << "DataFileHandler单元测试通过" << endl;
  return 0;
}
