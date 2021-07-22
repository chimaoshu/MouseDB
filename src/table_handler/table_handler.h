#ifndef MOUSEDB_SRC_TABLE_HANDLER_TABLE_HANDLER_H_
#define MOUSEDB_SRC_TABLE_HANDLER_TABLE_HANDLER_H_

#include <list>
#include <map>
#include <string>

// hot-dump需要使用子线程进行
#include <thread>

// json
#include "third_part/Json/single_include/nlohmann/json.hpp"

// 文件读写
#include "src/data_file_handler/data_file_handler.h"

// 表元数据管理
#include "src/table_meta_handler/table_meta_handler.h"

// 库元数据管理
#include "src/database_meta_handler/database_meta_handler.h"

// 冷热数据
#include "src/cold_hot_data/cold_data.h"
#include "src/cold_hot_data/hot_data.h"

// 对某个数据库中表的处理，包括表的创建、打开、删除
// 对于某个表的查询、插入、冷热数据的处理和转化也在这里完成
// TablesHandler连接起了表元数据与数据库元数据
// 与TableMetaHandler只管理一个表不同的是，该类管理某个数据库中的所有表
// 即一个TablesHandler实例将对应一个被打开的数据库
// 用户进入一个数据库时，一个TablesHandler将会被实例化
// 该类不应该被释放，除非您确定以后不会再使用该数据库
// 即使暂时不使用该数据库，也会被隐藏
class TablesHandler {

private:
  // 数据库头JSON，存储了所有的数据库和对应的表及其路径
  // 来自对DatabaseMetaHandler成员的引用
  // 格式参考format/database_meta.jsonc
  const nlohmann::json &database_meta_;

  // 当前打开的数据库名（一个该类的实例对应一个被打开的数据库）
  std::string database_name_;

  // 存储从表名到table_meta_handler的映射
  // 每次创建TablesHandler都会在此处缓存所有表对应的TableMetaHandler，方便读操作
  // 内容包括：表名及其对应元数据
  // 格式参考format/tables_info.jsonc
  std::map<std::string, TableMetaHandler *>
      map_of_table_name_to_table_meta_handler_;

  // 存储从表名到HotDataManager的映射
  std::map<std::string, HotDataManager *>
      map_of_table_name_to_hot_data_manager_;

  // 存储从表名到ColdData的映射
  std::map<std::string, ColdDataManager *>
      map_of_table_name_to_cold_data_manager_;

  // 返回对TableMetaHandler指针的引用，之所以要用引用是为了防止指针乱飞，双重释放等问题
  inline TableMetaHandler *&
  get_table_meta_handler(const std::string &table_name) {
    // 返回表名对应的指向TableMetaHandler实例的指针的引用
    return map_of_table_name_to_table_meta_handler_[table_name];
  }

  // 返回对DataManager的引用，若不存在，则创建存储后返回，在程序关闭之前不做删除
  HotDataManager *&get_hot_data_manager(const std::string &table_name);
  ColdDataManager *&get_cold_data_manager(const std::string &table_name);

  // 传入表名，修改对应map里面的DataManager为新的
  void set_hot_data_manager_in_map(const std::string &table_name,
                                   HotDataManager *new_hot_data_manager);
                                   
  void set_cold_data_manager_in_map(const std::string &table_name,
                                    ColdDataManager *new_cold_data_manager);

  StatusCode dump_table_in_sub_thread(const std::string &table_name);

public:
  // 构造函数，传入数据库元数据的JSON信息（databse_meta）
  // database_meta是对DatabaseMetaHandler成员的引用
  // 在DatabaseMetaHandler对JSON修改时，JSON也会同步得到修改
  TablesHandler(const std::string &database_name_,
                const nlohmann::json &database_meta);

  ~TablesHandler();

  // 查询 TODO
  std::list<nlohmann::json> query(const std::string &table_name);

  // 对某个table进行hot-dump操作
  StatusCode dump_table(const std::string &table_name);

  // 获得数据库名
  const std::string &get_database_name();

  // 传入表名、返回对应的table_meta的引用
  // 若不存在，返回空json
  const nlohmann::json &get_table_meta(const std::string &table_name);

  // 传入一个表名、获取这个表名对应的表的存储路径
  // 所有的冷、热数据的二进制文件都存储在这个dir中
  // 若表不存在，返回空字符串
  std::string get_table_dir(const std::string &table_name);

  // 创建一个表
  // table_directory：表的存储路径
  // table_name：表名
  // table_meta：表头
  // db_mete_handler：对数据库元数据进行操作
  StatusCode create_table(const std::string &table_directory,
                           const std::string &table_name,
                           nlohmann::json &table_meta,
                           DatabaseMetaHandler &db_mete_handler);

  // 传入表名与DatabaseMetaHandler实例
  // 删除一个表，所做的工作包括：
  // 在database_meta中抹除该表信息并保存、在磁盘上删除对应表路径下的JSON与MDB文件
  StatusCode drop_table(const std::string &table_name,
                         DatabaseMetaHandler &db_mete_handler);

  // 返回该数据库下所有表的表名
  std::list<std::string> get_all_table_names();
};

#endif // MOUSEDB_SRC_TABLE_HANDLER_H_