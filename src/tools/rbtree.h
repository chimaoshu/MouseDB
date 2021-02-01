#ifndef MOUSEDB_SRC_TOOLS_RBTREE_H_
#define MOUSEDB_SRC_TOOLS_RBTREE_H_

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "src/tools/type.h"

// 要求以int32为主键，方便进行检索
typedef int32_t primary_key_type;

// 假设一个文件中最多有2^32行
// 所以使用uint32来表示某行数据在文件中的第几行
typedef uint32_t row_order;

// 定义红黑树的key，对应每一行数据的primary key
// vector的size()就是primary key的数量
// 下表小的primary key有着更高的优先级
typedef std::vector<primary_key_type> rbtree_key;

// 红黑树的value，即一个节点的数据，有一个pair构成
// pair的第一个值为primary key，第一个值为数据在文件中的行数
typedef std::pair<rbtree_key, row_order> rbtree_value;
typedef rbtree_value rbtree_node;

// primary key的比较规则，若x小于y，返回真
// 比较规则：有限比较第一个，若相等则比较下一个
// 即：按照字典序比较大小
struct compare_rule_for_primary_key {
  inline bool operator()(const rbtree_key &x, const rbtree_key &y) const {
    // debug
    assert(x.size() == y.size());

    return x < y;

    // for (int i = 0; i < x.size(); i++)
    // {
    //     if (x[i] < y[i])
    //     {
    //         return true;
    //     }
    //     else if (x[i] > y[i])
    //     {
    //         return false;
    //     }
    // }
  }
};

// 定义一颗红黑树：以primary key为键，以数据在
// 文件中的行数为值，以两者构成的pair作为红黑树的节点
// 定义从节点得到primary key的规则是获取pair的第一个
// 定义节点的比较规则是上述定义的比较规则
typedef std::_Rb_tree<const rbtree_key, rbtree_node,
                      std::_Select1st<rbtree_node>,
                      compare_rule_for_primary_key>
    rbtree;

#endif