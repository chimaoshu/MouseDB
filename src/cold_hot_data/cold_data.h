#ifndef MOUSEDB_SRC_COLD_HOT_DATA_COLD_DATA_H_
#define MOUSEDB_SRC_COLD_HOT_DATA_COLD_DATA_H_

#include <vector>

#include "src/table_row_handler/table_row_handler.h"

class ColdDataManager
{
private:
    // 用于与冷数据文件进行交互
    TableRowHandler *cold_data_file_;

    // 表路径，即冷数据文件所在路径
    const std::string &table_dir_;

    // 索引，分块的索引，可以用二分法查找
    // first是主键，second是在文件中行数
    std::vector<rbtree_value> index_;

    // 文件的数据行数
    int row_number_of_cold_data_file_;

    // 相邻两处索引值之间间隔的行数
    const int index_interval_rows_number_;

    // 建立索引，赋值到index_
    // 即每128行为一块，每块用块头第一行表示
    status_code build_index();

    // 传入两个主键，获取处于两者之间的数据在文件中的行数（min<=output<=max），包括min==max的情况
    // TODO 以后加个min<answer<max
    // 目前是min<=answer<=max
    std::list<row_order> find_values_between_primary_keys(const rbtree_key &min, const rbtree_key &max);

public:
    // 读取冷数据文件数据，建立索引
    // 文件路径从table_meta_handler中获取
    // cold_data_file_name: 默认为空字符串，表示不使用传入的冷数据文件名
    // 将从TableMeta中获取冷数据文件名，创建冷数据文件
    // 若其不为空，表示使用传入的文件名创建冷数据文件
    ColdDataManager(const std::string &table_dir,
                    TableMetaHandler *&table_meta_handler,
                    const std::string cold_data_file_name = "",
                    uint8_t index_interval_rows_number = 128);

    ~ColdDataManager();

    // 搜索
    
};

#endif // MOUSEDB_SRC_COLD_HOT_DATA_COLD_DATA_H_