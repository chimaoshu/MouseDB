#include "database_meta_handler.h"
#include "src/exception_handler/exception_handler.h"

using namespace std;
using json = nlohmann::json;

DatabaseMetaHandler::DatabaseMetaHandler(const string &database_meta_dir,
                                         json &database_meta) {
  // data文件夹不存在，则创建
  if (!Tools::dir_or_file_exists(database_meta_dir)) {
    Tools::make_dir(database_meta_dir);
  }

  database_meta_path_ = database_meta_dir + "/db_meta.json";
  database_meta_ = database_meta;
}

DatabaseMetaHandler::DatabaseMetaHandler(const string &database_meta_dir) {
  // 文件夹不存在，则创建
  if (!Tools::dir_or_file_exists(database_meta_dir)) {
    Tools::make_dir(database_meta_dir);
  }

  database_meta_path_ = database_meta_dir + "/db_meta.json";
  load();
}

const string &DatabaseMetaHandler::get_database_meta_path() {
  return database_meta_path_;
}

const json &DatabaseMetaHandler::get_database_meta() { return database_meta_; }

// 使用传入的路径保存meta，即“另存为”
StatusCode DatabaseMetaHandler::save(const string &database_meta_path) {
  // TODO：检查数据库名、meta信息、路径是否非空

  DataFileHandler file;

  StatusCode err = file.open(database_meta_path, 1, 0, 1, 0);

  // 文件不存在
  if (err)
    return err;

  file.append(database_meta_.dump());

  return StatusCode::SUCCESS;
}

// 使用内置的成员变量存储（覆盖原文件数据）
StatusCode DatabaseMetaHandler::save() {
  // TODO：检查数据库名、meta信息、路径是否非空
  DataFileHandler file;

  StatusCode err = file.open(database_meta_path_, 1, 0, 1, 0);

  // 文件不存在
  if (err)
    return err;

  file.append(database_meta_.dump());

  return StatusCode::SUCCESS;
}

StatusCode DatabaseMetaHandler::load() {
  DataFileHandler file;

  // 数据库头文件不存在则创建
  if (!file.file_exist(database_meta_path_)) {
    // 创建文件
    StatusCode err = file.open(database_meta_path_);

    if (err)
      return err;

    // 写入空JSON
    file.append("{}");

    // 空JSON赋值
    database_meta_ = json::object();

    return StatusCode::SUCCESS;
  }

  // 以读取的模式打开
  StatusCode err = file.open(database_meta_path_, 0, 0, 0, 1);

  if (err)
    return err;

  respond<string> json_string = file.read_all_text();
  err = json_string.second;

  if (err) {
    return err;
  } else {
    database_meta_ = json::parse(json_string.first);
    return StatusCode::SUCCESS;
  }
}

StatusCode
DatabaseMetaHandler::add_database_in_json(const string &database_name) {
  // 检查数据库是否已经存在
  auto it = database_meta_.find(database_name);

  // 不存在
  if (it == database_meta_.end()) {
    database_meta_[database_name] = json::object();
    return StatusCode::SUCCESS;
  } else // 存在
  {
    return StatusCode::ERROR_DATABASE_HAS_ALREADY_EXISTS;
  }
}

StatusCode DatabaseMetaHandler::add_table_in_json(const string &database_name,
                                                   const string &table_name,
                                                   const string &table_dir) {
  // 检查表是否已经存在
  auto it = database_meta_.find(database_name);

  // 不存在
  if (it == database_meta_.end()) {
    return StatusCode::ERROR_TABLE_HAS_ALREADY_EXISTS;
  } else // 存在
  {
    database_meta_[database_name][table_name] = table_dir;
    return StatusCode::SUCCESS;
  }
}

StatusCode
DatabaseMetaHandler::delete_database_in_json(const string &database_name) {
  auto it = database_meta_.find(database_name);
  if (it == database_meta_.end()) {
    return StatusCode::ERROR_DATABASE_NOT_EXISTS;
  } else {
    database_meta_.erase(it);
    return StatusCode::SUCCESS;
  }
}

StatusCode
DatabaseMetaHandler::delete_table_in_json(const string &database_name,
                                          const string &table_name) {
  auto it = database_meta_[database_name].find(table_name);
  if (it == database_meta_[database_name].end()) {
    return StatusCode::ERROR_TABLE_NOT_EXISTS;
  } else {
    database_meta_[database_name].erase(it);
    return StatusCode::SUCCESS;
  }
}