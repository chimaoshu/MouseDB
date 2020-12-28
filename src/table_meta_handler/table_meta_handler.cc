#include "table_meta_handler.h"

using namespace std;
using json = nlohmann::json;

const string TableMetaHandler::get_table_name()
{
    return table_meta_["table_name"];
}

const string &TableMetaHandler::get_table_meta_path()
{
    return table_meta_path_;
}

const json &TableMetaHandler::get_table_meta()
{
    return table_meta_;
}

inline int TableMetaHandler::get_key_number()
{
    return table_meta_["primary_key"].size() + table_meta_["other_keys"].size();
}

inline int TableMetaHandler::get_primary_key_number()
{
    return table_meta_["primary_key"].size();
}

const string TableMetaHandler::get_current_used_hot_data_file_name()
{
    auto it = table_meta_["hot_data"].find("current_used");

    assert(it != table_meta_["hot_data"].end());

    if (*it == NULL)
        return "";
    else
        return *it;
}

const string TableMetaHandler::get_new_hot_data_file_name()
{
    auto it = table_meta_["hot_data"].find("new");

    assert(it != table_meta_["hot_data"].end());

    if (*it == NULL)
        return "";
    else
        return *it;
}

const std::string TableMetaHandler::get_current_used_cold_data_file_name()
{
    auto it = table_meta_["cold_data"].find("current_used");

    assert(it != table_meta_["cold_data"].end());

    if (*it == NULL)
        return "";
    else
        return *it;
}

const std::string TableMetaHandler::get_new_cold_data_file_name()
{
    auto it = table_meta_["cold_data"].find("new");

    assert(it != table_meta_["cold_data"].end());

    if (*it == NULL)
        return "";
    else
        return *it;
}

void TableMetaHandler::set_new_hot_data_file_name(const string &new_hot_data_file_name)
{
    table_meta_["hot_data"]["new"] = new_hot_data_file_name;
}

void TableMetaHandler::set_new_cold_data_file_name(const string &new_cold_data_file_name)
{
    table_meta_["cold_data"]["new"] = new_cold_data_file_name;
}

void TableMetaHandler::change_new_hot_data_file_to_current_used()
{
    // 顶替current_used
    string new_hot_data_file_name = table_meta_["hot_data"]["new"];
    table_meta_["hot_data"]["current_used"] = new_hot_data_file_name;

    // 原来位置还原为null
    table_meta_["hot_data"]["new"] = NULL;
}

void TableMetaHandler::change_new_cold_data_file_to_current_used()
{
    // 顶替current_used
    string new_cold_data_file_name = table_meta_["cold_data"]["new"];
    table_meta_["cold_data"]["current_used"] = new_cold_data_file_name;

    // 原来位置还原为null
    table_meta_["cold_data"]["new"] = NULL;

    // 添加到history中
    table_meta_["cold_data"]["history"].push_back(new_cold_data_file_name);
}

// 通过load读取对应路径下元数据
TableMetaHandler::TableMetaHandler(const string &table_meta_path)
    : table_meta_path_(table_meta_path)
{
    load();
}

// 直接传入元数据
TableMetaHandler::TableMetaHandler(const string &table_meta_path, json &table_meta)
    : table_meta_(table_meta),
      table_meta_path_(table_meta_path)
{
}

// 传入存储路径
status_code TableMetaHandler::save(const string &table_meta_path)
{
    DataFileHandler file;
    file.open(table_meta_path, 1, 0, 1, 0);
    return file.append(table_meta_.dump());
}

// 覆盖原文件存储
status_code TableMetaHandler::save()
{
    DataFileHandler file;
    file.open(table_meta_path_, 1, 0, 1, 0);
    return file.append(table_meta_.dump()); // 返回错误码
}

// 从磁盘中读取表头信息
status_code TableMetaHandler::load()
{
    DataFileHandler file;

    // 只读
    status_code err = file.open(table_meta_path_, 0, 0, 0, 1);

    // 一般是路径错误，文件打不开
    if (err)
    {
        cout << "warning:table meta json file not exists." << '\n';
        return err;
    }

    respond<string> p = file.read_all();

    if (!p.first.empty())
    {
        table_meta_ = json::parse(p.first);
        return error_code::SUCCESS;
    }
    else
    {
        // 返回错误码
        return p.second;
    }
}

uint16_t TableMetaHandler::get_line_size()
{
    // 若已有缓存，直接返回
    if (!off_set_of_each_column_.empty())
        return line_size_;

    set_column_info_and_line_size();
    return line_size_;
}

vector<uint16_t> &TableMetaHandler::get_off_set_of_each_column()
{
    // 若已有缓存，直接返回
    if (!off_set_of_each_column_.empty())
        return off_set_of_each_column_;

    set_column_info_and_line_size();
    return off_set_of_each_column_;
}

vector<int8_t> &TableMetaHandler::get_type_of_each_column()
{
    if (!type_of_each_column_.empty())
        return type_of_each_column_;

    set_column_info_and_line_size();
    return type_of_each_column_;
}

list<int> TableMetaHandler::get_column_orders_by_names(const list<string> &column_names)
{
    if (type_of_each_column_.empty())
        set_column_info_and_line_size();

    list<int> output;

    for (auto it = column_names.begin(); it != column_names.end(); it++)
    {
        // 加入列名对应的序号
        output.push_back(map_from_column_name_to_column_order_[*it]);
    }

    // 排序
    output.sort();

    return output;
}

status_code TableMetaHandler::set_column_info_and_line_size()
{
    // 若已有缓存，直接返回
    if (!off_set_of_each_column_.empty())
        return error_code::SUCCESS;

    json &primary_keys = table_meta_["primary_key"];
    json &other_keys = table_meta_["other_keys"];

    // 一共有这么多列（key）
    uint8_t key_number = primary_keys.size() + other_keys.size();

    // 新建一个数组用于存储每一个column（key）在row中的偏移位置
    // 首位置存column数量
    off_set_of_each_column_.resize(key_number + 1);
    off_set_of_each_column_[0] = key_number;
    off_set_of_each_column_[1] = 0;

    // 新建数组存储column中key的类型
    type_of_each_column_.resize(key_number + 1);
    type_of_each_column_[0] = key_number;

    // 清空
    if (!map_from_column_name_to_column_order_.empty())
    {
        map_from_column_name_to_column_order_.clear();
    }

    // 变量长度（所占字节数）
    int size_of_var = 0;

    // 行长度
    line_size_ = 0;

    // 从off_set_of_each_column_第二个元素开始赋值
    int i = 2;

    /* 类型与对应数字翻译表：
    1:int_8_t
    2:int_16_t
    3:int_32_t
    4:int_64_t
    5:uint_8_t
    6:uint_16_t
    7:uint_32_t
    8:uint_64_t
    9:float
    10:double
    -123:char（负数表示char，数值表示char的长度）（其实char的长度也可以通过前后偏移相减得到）*/

    // 先找primary keys
    for (auto it = primary_keys.begin(); it != primary_keys.end(); it++)
    {
        // 寻找key_type
        auto type_it = it->find("type");
        if (type_it == it->end())
        {
            return error_code::ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META;
        }

        // key的数据种类
        string key_type = *type_it;
        if (key_type == "int")
        {
            // 字节数
            size_of_var = (*it)["length"];

            // 转为正数（由于json中存储时用负号表示signed int）
            if (size_of_var < 0)
            {
                type_of_each_column_[i - 1] = size_of_var / -8;
            }
            else // 正号表示unsigned
            {
                type_of_each_column_[i - 1] = size_of_var / 8 + 4;
            }
        }
        else if (key_type == "char")
        {
            size_of_var = (*it)["length"];

            // 负号表示char，数值表示长度
            type_of_each_column_[i - 1] = -size_of_var;
        }
        else if (key_type == "float")
        {
            size_of_var = sizeof(float);
            type_of_each_column_[i - 1] = 9;
        }
        else if (key_type == "double")
        {
            size_of_var = sizeof(double);
            type_of_each_column_[i - 1] = 10;
        }
        else
        {
            return error_code::ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META;
        }

        // 设置从列名到列序号的映射，列序号从1开始
        map_from_column_name_to_column_order_[(*it)["name"]] = i - 1;

        // 加上当前key的类型对应的size
        line_size_ += size_of_var > 0 ? size_of_var : -size_of_var;

        // 第i个元素的地址偏移就等于前i-1个元素的size之和
        off_set_of_each_column_[i] = line_size_;
        i++;
    }

    // 找other keys
    for (auto it = other_keys.begin(); it != other_keys.end(); it++)
    {
        // 寻找key_type
        auto type_it = it->find("type");
        if (type_it == it->end())
        {
            return error_code::ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META;
        }

        // key的数据种类
        string key_type = *type_it;
        if (key_type == "int")
        {
            // 字节数
            size_of_var = (*it)["length"];

            // 转为正数（由于json中存储时用负号表示signed int）
            if (size_of_var < 0)
            {
                type_of_each_column_[i - 1] = size_of_var / -8;
            }
            else // 正号表示unsigned
            {
                type_of_each_column_[i - 1] = size_of_var / 8 + 4;
            }
        }
        else if (key_type == "char")
        {
            size_of_var = (*it)["length"];

            // 负号表示char，数值表示长度
            type_of_each_column_[i - 1] = -size_of_var;
        }
        else if (key_type == "float")
        {
            size_of_var = sizeof(float);
            type_of_each_column_[i - 1] = 9;
        }
        else if (key_type == "double")
        {
            size_of_var = sizeof(double);
            type_of_each_column_[i - 1] = 10;
        }
        else
        {
            return error_code::ERROR_KEY_TYPE_NOT_FOUND_IN_TABLE_META;
        }

        // 设置从列名到列序号的映射
        map_from_column_name_to_column_order_[(*it)["name"]] = i - 1;

        // 加上当前key的类型对应的size
        line_size_ += size_of_var > 0 ? size_of_var : -size_of_var;

        // 第i个元素的地址偏移就等于前i-1个元素的size之和
        off_set_of_each_column_[i] = line_size_;
        i++;
    }

    return error_code::SUCCESS;
}