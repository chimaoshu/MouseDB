#ifndef MOUSEDB_SRC_TABLE_BUFFER_READER_AND_WRITER_H_
#define MOUSEDB_SRC_TABLE_BUFFER_READER_AND_WRITER_H_

#include <string>
#include <vector>

#include "src/tools/type.h"

#include "third_part/Json/single_include/nlohmann/json.hpp"

/*
这代码早晚要被优化掉
递归效率太低，况且这段代码会被重复调用
*/

// 传入：种类ID、初始写入地址、存入的容器（JSON）
// 如果要高性能，应该用protobuf而不是json传递信息的。。。以后慢慢改吧。
template <int N>
inline void read_buffer(const int &type_id, void *&initial_address, nlohmann::json &container)
{
    if (type_id == N)
    {
        container.push_back(*(KeyType<N> *)initial_address);
    }
    else
    {
        read_buffer<N - 1>(type_id, initial_address, container);
    }
}

template <>
inline void read_buffer<0>(const int &type_id, void *&initial_address, nlohmann::json &container)
{
    std::string s;
    int lenght_of_char = -type_id;

    // 一个字节一个字节读出
    for (int i = 0; i < lenght_of_char; i++)
    {
        s.push_back(*(char *)(initial_address) + i);
    }

    container.push_back(s);
}

// 传入：种类ID、初始写入地址、JSON迭代器（对应列的元素）
template <int N>
inline void write_buffer(const int &type_id, void *&initial_address, nlohmann::detail::iter_impl<const nlohmann::json> &it)
{
    if (type_id == N)
    {
        KeyType<N> temp = *it;
        *(KeyType<N> *)initial_address = temp;
    }
    else
    {
        write_buffer<N - 1>(type_id, initial_address, it);
    }
}

template <>
inline void write_buffer<0>(const int &type_id, void *&initial_address, nlohmann::detail::iter_impl<const nlohmann::json> &it)
{
    std::string temp = *it;

    // 一个一个字节写入
    int off_set_size = 0;
    for (auto it = temp.begin(); it < temp.end(); it++, off_set_size++)
    {
        *((char *)(initial_address) + off_set_size) = *it;
    }
}

#endif // MOUSEDB_SRC_TABLE_BUFFER_READER_AND_WRITER_H_