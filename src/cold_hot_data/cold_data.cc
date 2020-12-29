#include "src/cold_hot_data/cold_data.h"

// 建立索引，赋值到index_
// 即每128行为一块，每块用块头第一行表示
status_code ColdDataManager::build_index()
{
    assert(cold_data_file_ != NULL);

    row_number_of_cold_data_file_ = cold_data_file_->get_number_of_rows_in_file();

    // 预分配内存，并预设值rbtree_value值
    // 比如一共有515行，设定每128行一个索引，那么索引分别取第1、129、257、385、513行

    // 如果正好整除，则索引行数等于总行数除以间隔数
    if (row_number_of_cold_data_file_ % index_interval_rows_number_ == 0)
        index_.resize(row_number_of_cold_data_file_ / index_interval_rows_number_, rbtree_value());

    // 如果不整除，则索引行数等于总行数除以间隔数加一
    else
        index_.resize(row_number_of_cold_data_file_ / index_interval_rows_number_ + 1, rbtree_value());

    // 建索引
    rbtree_key *primary_key;
    order_of_row_in_file order_to_read = 1;
    for (int i = 0; i < index_.size(); i++)
    {
        index_[i].second = order_to_read;

        primary_key = cold_data_file_->read_row_index(order_to_read);
        assert(primary_key != NULL);

        index_[i].first = *primary_key;
        delete primary_key;

        order_to_read += 128;
    }
}

// 读取冷数据文件数据，建立索引
// 文件路径从table_meta_handler中获取
// cold_data_file_name: 默认为空字符串，表示不使用传入的冷数据文件名
// 将从TableMeta中获取冷数据文件名，创建冷数据文件
// 若其不为空，表示使用传入的文件名创建冷数据文件
ColdDataManager::ColdDataManager(const string &table_dir,
                                 TableMetaHandler *&table_meta_handler,
                                 const string cold_data_file_name = "",
                                 uint8_t index_interval_rows_number = 128)
    : table_dir_(table_dir_),
      index_interval_rows_number_(index_interval_rows_number)
{

    string cold_data_file_path;

    if (cold_data_file_name.empty())
        cold_data_file_path = table_dir + '/' + table_meta_handler->get_current_used_cold_data_file_name();
    else
        cold_data_file_path = table_dir + '/' + cold_data_file_name;

    cold_data_file_ = new TableRowHandler(cold_data_file_path, table_meta_handler, 0, 1);

    this->build_index();
}

ColdDataManager::~ColdDataManager()
{
    delete cold_data_file_;
}