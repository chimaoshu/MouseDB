#ifndef MOUSEDB_SRC_TOOLS_TYPE_H_
#define MOUSEDB_SRC_TOOLS_TYPE_H_


// support diffrent type of number
#include <stdint.h>

// use the memcpy function in it
#include<string.h>

#include "src/exception_handler/exception_handler.h"

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

template <int N> struct KeyTypeMap{ using Type = char; };
template <> struct KeyTypeMap<1>{ using Type = int8_t; };
template <> struct KeyTypeMap<2>{ using Type = int16_t; };
template <> struct KeyTypeMap<3>{ using Type = int32_t; };
template <> struct KeyTypeMap<4>{ using Type = int64_t; };
template <> struct KeyTypeMap<5>{ using Type = uint8_t; };
template <> struct KeyTypeMap<6>{ using Type = uint16_t; };
template <> struct KeyTypeMap<7>{ using Type = uint32_t; };
template <> struct KeyTypeMap<8>{ using Type = uint64_t; };
template <> struct KeyTypeMap<9>{ using Type = float; };
template <> struct KeyTypeMap<10>{ using Type = double; };


template <int N>
using KeyType = typename KeyTypeMap<N>::Type;

// storage the value and type of a key
class key_with_type_info
{
public:

    // key种类的编码
    int8_t code_of_key_type_;

    // 数据大小
    int8_t data_size_;

    // 指向数据的内存
    void *data_;

    // 构造函数，传入key的种类和key的数据地址
    key_with_type_info(int8_t code_of_key_type, void* pointer_to_input_data)
    {
        code_of_key_type_ = code_of_key_type;     

        switch (code_of_key_type)
        {
        // 4Byte
        case 3:
        case 7:
            data_size_ = 4;
            break;

        // 1Byte
        case 1:
        case 5:
            data_size_ = 1;
            break;
        
        // 2Byte
        case 2:
        case 6:
            data_size_ = 2;
            break;

        // 4Byte
        case 4:
        case 8:
            data_size_ = 4;
            break;

        // char array
        default:
            assert(code_of_key_type < 0);
            data_size_ = -code_of_key_type;
            break;
        }   

        // allocate memory
        data_ = new char[data_size_];

        // copy
        memcpy(data_, pointer_to_input_data, data_size_);
    }

    ~key_with_type_info()
    {
        delete[] (char *)data_;
    }

    // 在外面先根据code得到种类，然后传进来赋值
    // 字符数组需要在后面自行补\0
    void get_data(void* dest)
    {
        memcpy(dest, data_, data_size_);
    }
};


union key_without_type_info
{
    uint8_t ui8;
    int8_t i8;

    uint16_t ui16;
    int16_t i16;

    uint32_t ui32;
    int32_t i32;

    uint64_t ui64;
    int64_t i64;

    char ch[64];
};

#endif // MOUSEDB_SRC_TOOLS_TYPE_H_