#include "src/cold_hot_data/hot_data.h"

using namespace std;
using json = nlohmann::json;

// 最后一个参数：是否为hot_dump创建的，如果是hot-dump创建的，则不考虑对TableMeta进行检查
// 如果不是hot-dump创建的，说明是数据库第一次打开，那么需要见查看TableMeta看热数据是否已经全部转化为冷数据
HotDataManager::HotDataManager(const string &table_dir,
                               TableMetaHandler *&table_meta_handler,
                               bool hot_dump)
    : table_meta_handler_(table_meta_handler), table_dir_(table_dir) {

  string current_used_hot_data_file_path =
      table_meta_handler_->get_current_used_hot_data_file_name();

  string new_hot_data_file_path =
      table_meta_handler_->get_new_hot_data_file_name();

  // 如果上次关闭程序时正常关闭，那么所有的热数据都会dump成冷数据，Meta中对应位置为null
  // 返回得到的字符串就为空字符串
  if (current_used_hot_data_file_path.empty() || hot_dump) {

    // 若使用热数据文件为空，说明上次数据库正常退出
    assert(new_hot_data_file_path.empty());

    // 正常创建数据库
    string timestamp = Tools::get_str_timestamp();
    string new_hot_data_file_path = table_dir_ + '/' + timestamp + "hot";

    // 启用缓存，这个manager将在hot-dump完成后成为新的manager
    current_used_hot_data_file_ =
        new TableRowHandler(new_hot_data_file_path, table_meta_handler_, 1, 1);

    // 存储到Meta中
    table_meta_handler_->set_new_hot_data_file_name(timestamp + ".hot");
    table_meta_handler_->save();

    // 记录热数据文件中总共有多少行数据
    row_number_in_hot_data_file_ = 0;

    return;
  }
  // 如果上次关闭程序时异常，那么current_used_hot_data_file_path不为空
  // 如果这个异常还是在hot-dump过程中发生的，那么new_hot_data_file_path也不为空
  // 这里需要讨论这两种情况的数据恢复问题
  else {

    // 在非hot-dump过程中出现问题，只需在内存中恢复有序索引即可
    if (new_hot_data_file_path.empty()) {

      // 开一个TableRowHandler把hot文件中的数据全部读到红黑树，不销毁，进行预读
      current_used_hot_data_file_ = new TableRowHandler(
          current_used_hot_data_file_path, table_meta_handler_, 0, 1);

      // 记录热数据文件中有多少行数据
      row_number_in_hot_data_file_ = 0;

      // 读取下一行的rbtree_key
      rbtree_key *next_row_key = current_used_hot_data_file_->read_row_index();

      // 循环写入红黑树
      while (next_row_key != NULL) {
        this->add_node(*next_row_key, ++row_number_in_hot_data_file_);
        next_row_key = current_used_hot_data_file_->read_row_index();
      }

      return;
    }
    // 在hot-dump过程中出现问题，那么不仅要重新dump，还要恢复新热数据的索引
    // 需要：
    // 1、重新进行执行出现意外的hot-dump（先恢复索引，步骤同上，然后进行hot-dump）
    // 2、把新的manager变为正在使用的manager，在内存中恢复有序索引（步骤同上）
    else {

      // 没dump成功的文件
      TableRowHandler data_file_to_dump(current_used_hot_data_file_path,
                                        table_meta_handler_, 0, 1);

      // 读到红黑树中

      // 记录热数据文件中有几行数据
      row_number_in_hot_data_file_ = 0;

      // 读取下一行的rbtree_key
      rbtree_key *next_row_key = data_file_to_dump.read_row_index();

      // 循环写入红黑树
      while (next_row_key != NULL) {
        this->add_node(*next_row_key, ++row_number_in_hot_data_file_);
        next_row_key = data_file_to_dump.read_row_index();
      }

      // 生成冷数据文件，此时冷数据文件已经被设置为new
      this->dump_to_cold_data(true);

      // 把冷数据文件从new转为current_used
      table_meta_handler_->change_new_cold_data_file_to_current_used();

      // 清空红黑树
      tree_.clear();

      // 创建新的热数据的TableRowHandler
      current_used_hot_data_file_ = new TableRowHandler(
          new_hot_data_file_path, table_meta_handler_, 1, 1);

      // 把“新热数据文件”读取到内存中的红黑树索引

      // 新的热数据文件，有0行数据
      row_number_in_hot_data_file_ = 0;
      next_row_key = current_used_hot_data_file_->read_row_index();

      // 循环写入红黑树
      while (next_row_key != NULL) {
        this->add_node(*next_row_key, ++row_number_in_hot_data_file_);
        next_row_key = current_used_hot_data_file_->read_row_index();
      }

      // 把“新热数据”，也就是我们刚刚把索引读进内存的热数据文件设置为“正常使用”
      table_meta_handler_->change_new_hot_data_file_to_current_used();

      // 是不是要考虑一下重设cold_data的问题？或者设置cold_data在这时候读就可以自适应了
      return;
    }
  }
}

HotDataManager::~HotDataManager() {
  // 不用释放new_hot_data_manager_的内存
  delete current_used_hot_data_file_;
}

// 后续应该支持使用protobuf而不是json这种及其浪费资源的东西
// 传入参数不为const是因为要在循环前对引用single_row预留空间
status_code HotDataManager::insert_rows(const json &rows_info) {

  // 先写入文件
  status_code err = current_used_hot_data_file_->serialize_and_write(rows_info);

  // 再存到红黑树
  if (!err) {
    int primary_key_number = table_meta_handler_->get_primary_key_number();

    // 预留空间，防止在循环中反复分配
    rbtree_key key;
    key.resize(primary_key_number);

    // 预留空间
    auto it_of_key = rows_info.begin();

    // 遍历每一行数据
    for (auto it_of_rows = rows_info.begin(); it_of_rows != rows_info.end();
         it_of_rows++) {

      it_of_key = (*it_of_rows).begin();

      // 填入三个priamry key的值
      for (int i = 0; i < primary_key_number; i++) {
        key[i] = *it_of_key;
        it_of_key++;
      }

      // 添加至红黑树
      this->add_node(key, ++row_number_in_hot_data_file_);
    }
  }
  return err;
}

// TODO 查找，并进行索引优化

// 到时候看一下是否某些类内部使用的数据库的路径是否需要改，不然会出bug
// hot-dump的过程：
// 1、创建新的热数据manager，创建时会在TableMeta标记这个“新的热数据文件”。
// 2、将this->is_dumping = true，此后插入的数据会直接插入到新的HotDataManager中
//    此后查询的数据会在新、旧两份热数据和冷数据三者之间进行查询，前两个归并后和第三个归并。
// 3、创建一个新的冷数据的TableRowHandler，并在TableMeta中标记这个“新的冷数据文件”。
// 4、对当前的热数据文件和冷数据文件进行归并操作，形成一份新的冷数据，然后返回新的热数据manager。
// 5、TablesHandler中的函数拿到新的热数据manager后，再通过TableMeta中的记录，获取新的冷数据manager。
// 6、启动一个锁，即把老的HotDataManager的is_switching=true，这个锁会使得暂时无法进行查询和插入操作。
// 7、把TableMeta中的两个“新数据文件”，更改成“正在使用的数据文件”，即“转正”，并把原来的“新文件”字段置为null。
// 8、把两个map中的旧热数据manager和旧冷数据manager删除，替换为新的，此时已经可以进行插入和查询了，自适应。
//
// 关于数据恢复：
// 1、如果程序正常关闭，会把所有热数据全部dump成冷数据再关闭，此时会把TableMeta中的“正在使用热数据文件”字段置为null。
// 2、如果打开TableMeta发现“正在使用热数据文件”不为null，说明上次意外地被关闭了，所以需要读取热数据文件恢复内存中地索引。
// 3、如果打开TableMeta发现“正在使用热数据文件”不为null，并且“新数据”也不为空，说明上次是再hot-dump的过程中意外关闭，
//    那么首先要把dump失败的数据dump完（这里使用主线程，不用子线程），然后再恢复“新数据文件”在内存中的索引，并把它转正。
//
// 该函数：
// 把冷热数据文件dump成一份新的冷数据，并返回新的HotDataManager
// 参数：is_recovery若置为true，则说明是恢复数据的情况（上述第3点），则不开一个新的热数据manager
HotDataManager *HotDataManager::dump_to_cold_data(bool is_recovery) {

  // 运行在子线程中

  // 准备工作

  // 非数据恢复，则创建新的manager
  if (!is_recovery) {
    this->new_hot_data_manager_ =
        new HotDataManager(table_dir_, table_meta_handler_);
  }

  // 相当于一个锁，设置为true后
  // 查询操作将会从两份热数据和一个份数据之间做三路归并
  this->is_dumping = true;

  // 从TableMeta中得到现在正在使用的冷数据的路径
  string current_used_cold_data_file_path =
      table_dir_ + '/' +
      table_meta_handler_->get_current_used_cold_data_file_name();

  // 旧的冷数据文件需要读取，打开时不进行销毁，因为是顺序读，所以设置缓存页数为1，相当于预读一页
  TableRowHandler old_cold_data_file(current_used_cold_data_file_path,
                                     table_meta_handler_, false, 1);

  // 新的冷数据文件需要写入，打开时若文件存在则进行销毁，不进行缓存
  string new_cold_data_file_name =
      table_dir_ + Tools::get_str_timestamp() + ".mdb";
  TableRowHandler new_cold_data_file(new_cold_data_file_name,
                                     table_meta_handler_, true, 0);

  // 存储到TableMeta中
  table_meta_handler_->set_new_cold_data_file_name(new_cold_data_file_name);
  table_meta_handler_->save();

  // 主键比较规则
  compare_rule_for_primary_key is_less;

  // 红黑树的迭代器，其first为主键构成的vector
  // 其second为数据在热数据文件中位置
  auto it_of_rbtree = tree_.begin();
  auto end_of_rbtree = tree_.end();

  // 旧冷数据中的一行，其first为内存，second为主键构成的vector，两者使用完都要释放
  pair<void *, rbtree_key *> row_of_cold_data =
      old_cold_data_file.read_next_row_buffer_and_index();

  // 用于指向热数据某行内存的指针，这里预分配内存
  void *pointer_to_the_row_of_hot_data_file = NULL;

  // 归并操作，直到有一边到达末尾
  while (row_of_cold_data.first != NULL && it_of_rbtree != end_of_rbtree) {

    // 冷数据比较小，写入冷数据
    if (is_less(*(row_of_cold_data.second), it_of_rbtree->first)) {

      // 写入
      new_cold_data_file.write_rows(row_of_cold_data.first);

      // 释放数据内存空间
      delete[](char *) row_of_cold_data.first;

      // 释放主键
      delete row_of_cold_data.second;

      // 读取下一行冷数据
      row_of_cold_data = old_cold_data_file.read_next_row_buffer_and_index();

    }

    // 热数据比较小，写入热数据
    else {

      // 拿着红黑树上记录的，热数据在第几行，去热数据文件中读取并赋值
      pointer_to_the_row_of_hot_data_file =
          current_used_hot_data_file_->read_row_buffer(it_of_rbtree->second);

      // 写入新的冷数据
      new_cold_data_file.write_rows(pointer_to_the_row_of_hot_data_file);

      // 释放数据的内存
      delete[](char *) pointer_to_the_row_of_hot_data_file;

      // 读取下一个热数据
      it_of_rbtree++;
    }
  }

  // 若冷数据文件中无内容了
  if (row_of_cold_data.first == NULL) {

    // 直接把热数据全部写入文件
    while (it_of_rbtree != end_of_rbtree) {

      // 读取对应行的数据（一块内存）
      pointer_to_the_row_of_hot_data_file =
          current_used_hot_data_file_->read_row_buffer(it_of_rbtree->second);

      // 写入
      new_cold_data_file.write_rows(pointer_to_the_row_of_hot_data_file);

      // 释放内存
      delete[](char *) pointer_to_the_row_of_hot_data_file;
    }
  }

  // 若热数据中无内容了
  if (it_of_rbtree == end_of_rbtree) {

    // 直接简单粗暴冷数据读到文件末尾，不再使用read_next_row一行一行读了
    auto the_rest_of_the_file =
        old_cold_data_file.read_buffer_and_index_to_the_end();

    // 一整段加到新的冷数据文件中
    if (the_rest_of_the_file.second != 0)
      new_cold_data_file.write_rows(the_rest_of_the_file.first,
                                    the_rest_of_the_file.second);

    // 释放空间
    delete[](char *) the_rest_of_the_file.first;
  }

  // 返回新的manager
  return new_hot_data_manager_;
}

// 转变为switch模式意味着该实例不再接受查询操作
void HotDataManager::change_to_switch_mode() { this->is_switching = true; }

// TODO 以后加个min<answer<max
// 目前是min<=answer<=max
list<row_order>
HotDataManager::find_values_by_primary_keys(const rbtree_key &key) {

  list<row_order> values;

  auto res = tree_.equal_range(key);
  for (auto it = res.first; it != res.second; it++) {
    values.push_back((*it).second);
  }

  // ROV
  return values;
}

list<row_order>
HotDataManager::find_values_between_primary_keys(const rbtree_key &min,
                                                 const rbtree_key &max) {
  // debug
  assert(max.size() == min.size() && compare_rule_for_primary_key()(min, max));

  list<row_order> values;

  // 第一个大于等于min的值
  auto begin_it = tree_.lower_bound(min);

  // 第一个大于max的值
  auto end_it = tree_.upper_bound(max);

  for (; begin_it != end_it; begin_it++) {
    values.push_back((*begin_it).second);
  }

  // ROV
  return values;
}