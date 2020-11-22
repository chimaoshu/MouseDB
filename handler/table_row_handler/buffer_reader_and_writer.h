#ifndef MOUSEDB_HANDLER_TABLE_BUFFER_READER_AND_WRITER_H_
#define MOUSEDB_HANDLER_TABLE_BUFFER_READER_AND_WRITER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "lib/Json/single_include/nlohmann/json.hpp"

/*
负责处理从id到类型的映射
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
-123:char（负数表示char，数值表示char的长度）
*/

template <int N> struct BufferTypeMap{ using Type = char; };
template <> struct BufferTypeMap<1>{ using Type = int8_t; };
template <> struct BufferTypeMap<2>{ using Type = int16_t; };
template <> struct BufferTypeMap<3>{ using Type = int32_t; };
template <> struct BufferTypeMap<4>{ using Type = int64_t; };
template <> struct BufferTypeMap<5>{ using Type = uint8_t; };
template <> struct BufferTypeMap<6>{ using Type = uint16_t; };
template <> struct BufferTypeMap<7>{ using Type = uint32_t; };
template <> struct BufferTypeMap<8>{ using Type = uint64_t; };
template <> struct BufferTypeMap<9>{ using Type = float; };
template <> struct BufferTypeMap<10>{ using Type = double; };

template <int N>
using BufferType = typename BufferTypeMap<N>::Type;

// 传入：种类ID、初始写入地址、存入的容器
template <int N>
inline void read_buffer(const int &type_id, void *&initial_address, nlohmann::json &container)
{
    if (type_id == N)
    {
        container.push_back(*(BufferType<N> *)initial_address);
    }
    else
    {
        read_buffer<N - 1>(type_id, initial_address, container);
    }
}

template <>
inline void read_buffer<0>(const int &type_id, void *&initial_address, nlohmann::json &container)
{
    // char型
    if (type_id < 0)
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
    else
    {
        // 传入的参数错了
    }
}

// 传入：种类ID、初始写入地址、JSON迭代器（对应列的元素）
template <int N>
inline void write_buffer(const int type_id, void *initial_address, json row, int index)
{
    if (type_id == N)
    {
        BufferType<N> temp = row[index];
        *(BufferType<N> *)initial_address = temp;
    }
    else
    {
        write_buffer<N - 1>(type_id, initial_address, row, index);
    }
}

template <>
inline void write_buffer<0>(const int type_id, void *initial_address, json row, int index)
{
    // char型
    if (type_id < 0)
    {
        std::string temp = row[index];

        // 一个一个字节写入
        int off_set_size = 0;
        for (auto it = temp.begin(); it < temp.end(); it++, off_set_size++)
        {
            *((char *)(initial_address) + off_set_size) = *it;
        }
    }
    else
    {
        // 传入的参数错了
    }
}

#endif // MOUSEDB_HANDLER_TABLE_BUFFER_READER_AND_WRITER_H_