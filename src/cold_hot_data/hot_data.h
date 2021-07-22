#ifndef MOUSEDB_SRC_COLD_HOT_DATA_H_
#define MOUSEDB_SRC_COLD_HOT_DATA_H_

#include <string>

#include "src/table_row_handler/table_row_handler.h"
#include "src/tools/rbtree.h"
#include "src/tools/tools.h"

class HotDataManager {

private:
  // 红黑树
  rbtree tree_;

  // 用于进行二进制热数据文件的读写操作
  TableRowHandler *current_used_hot_data_file_ = NULL;

  // 对表元数据指针的引用
  TableMetaHandler *&table_meta_handler_;

  // 记录热数据中有几行数据
  row_order row_number_in_hot_data_file_;

  // 新的HotDataManager，用于存储hot-dump过程中到来的数据
  // 在dump完成后该实例将接替this成为新的HotDataManager
  HotDataManager *new_hot_data_manager_ = NULL;

  // 记录是否正在进行dump操作
  bool is_dumping = false;

  // hot-dump完成时切换数据库
  bool is_switching = false;

  // 表的冷、热数据所在的文件目录
  const std::string &table_dir_;

  // 传入primary key和对应行在文件中的行数，构成一个节点添加到红黑树
  inline void add_node(const rbtree_key &key, row_order row_order) {
    tree_._M_emplace_equal(key, row_order);
  }

  // 传入两个key值，返回所有在两者中间的节点的值组成的链表，用完需要自行释放空间
  // 这里不做索引优化，纯查找位于两个pk之间的值，需保证传入的参数符合primary
  // key格式
  std::list<row_order> find_values_between_primary_keys(const rbtree_key &min,
                                                        const rbtree_key &max);

  // 寻找primary key等于给定key值的所有值组成的链表，用完需要自行释放空间
  // 适用于where id = 的情况
  std::list<row_order> find_values_by_primary_keys(const rbtree_key &key);

  // TODO 删除操作

public:
  // table_dir: 表格所在目录
  // table_meta_handler: 表对应的元数据处理类
  // hot_dump:
  // 是否为hot_dump过程创建的临时HotDataManager，如果是，则不考虑对TableMeta进行检查
  // 如果不是hot-dump创建的，说明是数据库第一次打开，那么需要对TableMeta进行检查，检查如下：
  // 1、如果程序正常关闭，会把所有热数据全部dump成冷数据再关闭，此时会把TableMeta中的“正在使用热数据文件”字段置为null。
  //    那么程序会新建一个新的热数据文件。
  // 2、如果打开TableMeta发现“正在使用热数据文件”不为null，说明上次意外地被关闭了，所以需要读取热数据文件恢复内存中的索引。
  // 3、如果打开TableMeta发现“正在使用热数据文件”不为null，并且“新数据”也不为空，说明上次是再hot-dump的过程中意外关闭，
  //    那么首先要把dump失败的数据dump完（这里使用主线程，不用子线程），然后再恢复“新数据文件”在内存中的索引，并把它转正。
  HotDataManager(const std::string &table_dir,
                 TableMetaHandler *&table_meta_handler, bool hot_dump = false);

  ~HotDataManager();

  // TODO
  std::list<row_order> query();

  // 到时候看一下是否某些类内部使用的数据库的路径是否需要改，不然会出bug
  // hot-dump的过程：
  // 1、创建新的热数据manager，创建时会在TableMeta标记这个“新的热数据文件”。
  // 2、将this->is_dumping =
  // true，此后插入的数据会直接插入到新的HotDataManager中
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
  HotDataManager *dump_to_cold_data(bool is_recovery = false);

  // 转变为switch模式意味着该实例不再接受查询操作
  void change_to_switch_mode();

  // 后续应该支持使用protobuf而不是json这种及其浪费资源的东西
  // 传入参数不为const是因为要在循环前对引用single_row预留空间
  StatusCode insert_rows(const nlohmann::json &rows_info);

  // 删除数据
  StatusCode delete_rows();

  // 修改数据
  StatusCode update_rows();

  // TablesHandler在接收到查询请求时会调用该函数进行检查
  // 若已经在切换模式，则将查询请求缓存到消息队列中，等待切换完成后使用新的一套冷热数据进行查询
  inline bool is_switch_mode() { return this->is_switching; }
};

#endif // MOUSEDB_SRC_COLD_HOT_DATA_H_