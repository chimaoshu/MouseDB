#include "src/cold_hot_data/cold_data.h"
#include "src/exception_handler/exception_handler.h"

using namespace std;

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
    row_order order_to_read = 1;
    for (int i = 0; i < index_.size(); i++)
    {
        index_[i].second = order_to_read;

        primary_key = cold_data_file_->read_row_index(order_to_read);
        assert(primary_key != NULL);

        index_[i].first = *primary_key;
        delete primary_key;

        order_to_read += 128;
    }

    return status::SUCCESS;
}

// 读取冷数据文件数据，建立索引
// 文件路径从table_meta_handler中获取
// cold_data_file_name: 默认为空字符串，表示不使用传入的冷数据文件名
// 将从TableMeta中获取冷数据文件名，创建冷数据文件
// 若其不为空，表示使用传入的文件名创建冷数据文件
ColdDataManager::ColdDataManager(const string &table_dir,
                                 TableMetaHandler *&table_meta_handler,
                                 const string cold_data_file_name,
                                 uint8_t index_interval_rows_number)
    : table_dir_(table_dir),
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

// TODO 以后加个min<answer<max，需要连同前面的start_row、end_row一起改
// 目前是min <= answer <= max
// 返回符合条件的行在冷数据文件中的行数
list<row_order> ColdDataManager::find_values_between_primary_keys(const rbtree_key &min, const rbtree_key &max)
{
    row_order start_scanning_row = -1, end_scanning_row = -1;

    // 其实二分查找的过程应该可以用lower_bound和upper_bound来做的
    // 由于允许主键相同的数据出现，因此必须从主键小于min的第一个索引行开始扫描
    // 先判断特殊情况，是否直接从第一行开始扫描
    if (min <= index_[0].first)
    {
        start_scanning_row = 1;
    }
    // 是否从最后一个索引行开始扫描
    else if (min > index_.back().first)
    {
        start_scanning_row = index_.back().second;
    }
    // 二分查找，找到小于min的第一个索引行
    else
    {
        int low = 0, high = index_.size() - 1, mid;

        // 找到主键小于min的第一个索引行
        while (low <= high)
        {
            mid = (low + high) / 2;

            // 小于min
            if (index_[mid].first < min)
            {
                // mid < min <= mid + 1，从mid位置的索引开始扫描
                if (index_[mid + 1].first >= min)
                {
                    start_scanning_row = index_[mid - 1].second;
                    break;
                }
                else
                {
                    // 往主键更大的地方寻找
                    low = mid + 1;
                }
            }
            // 大于min
            else
            {
                // 往主键更小的地方寻找
                high = mid - 1;
            }
        }

        assert(start_scanning_row >= 0);
    }

    // 由于允许主键相同的数据出现，因此必须在主键大于max的第一个索引行停止扫描
    // 先判断特殊情况，是否直接从第一行停止扫描
    if (max < index_[0].first)
    {
        end_scanning_row = 1;
    }
    // 是否扫描到表末尾最后一行
    else if (max >= index_.back().first)
    {
        end_scanning_row = this->row_number_of_cold_data_file_;
    }
    // 二分查找，找到大于max的第一个索引行
    else
    {
        int low = 0, high = index_.size() - 1, mid;

        // 找到主键大于max的第一个索引行
        while (low <= high)
        {
            mid = (low + high) / 2;

            // 大于max
            if (index_[mid].first > max)
            {
                // mid-1 < max < mid，从mid位置的索引停止扫描
                if (index_[mid - 1].first <= max)
                {
                    end_scanning_row = index_[mid - 1].second;
                    break;
                }
                else
                {
                    // 往主键更小的地方寻找
                    high = mid - 1;
                }
            }
            // 小于max
            else
            {
                // 往主键更大的地方寻找
                low = mid + 1;
            }
        }

        assert(end_scanning_row >= 0);
    }

    // 读取对应位置的行的主键，检查是否符合条件
    rbtree_key *primary_key_of_current_row = NULL;

    // 返回符合条件的行在文件中的行数
    list<row_order> output;

    // 表示：找到第一个符合条件的min行，开始进行添加
    bool start_append_min = false;

    for (row_order current_row = start_scanning_row; current_row <= end_scanning_row; current_row++)
    {
        primary_key_of_current_row = cold_data_file_->read_row_index(current_row);

        if (!start_append_min)
        {
            // 找到第一个符合条件的min行，开始进行添加
            if (*primary_key_of_current_row == min)
            {
                start_append_min = true;
                output.push_back(current_row);
            }
            else
            {
                continue;
            }
        }
        else
        {
            if (*primary_key_of_current_row <= max)
            {
                output.push_back(current_row);
            }
            else
            {
                // 扫描到第一个大于max的则停止
                break;
            }
        }
    }

    // 此处触发RVO机制
    return output;
}