#ifndef MOUSEDB_SRC_TOOLS_RBTREE_H_
#define MOUSEDB_SRC_TOOLS_RBTREE_H_

#include <map>
#include <vector>
#include <list>
#include <iostream>

#include "src/tools/type.h"

// 要求以int32为主键，方便进行检索
typedef int32_t primary_key_type;

// 假设一个文件中最多有2^32行
// 所以使用uint32来表示某行数据在文件中的第几行
typedef uint32_t order_of_row_in_file;

// 定义红黑树的key，对应每一行数据的primary key
// vector的size()就是primary key的数量
// 下表小的primary key有着更高的优先级
typedef std::vector<primary_key_type> rbtree_key;

// 红黑树的value，即一个节点的数据，有一个pair构成
// pair的第一个值为primary key，第一个值为数据在文件中的行数
typedef std::pair<rbtree_key, order_of_row_in_file> rbtree_value;
typedef rbtree_value rbtree_node;

// primary key的比较规则，若x小于y，返回真
// 比较规则：有限比较第一个，若相等则比较下一个
struct compare_rule_for_primary_key
{
    inline bool
    operator()(const rbtree_key &x, const rbtree_key &y) const;
};

// 定义一颗红黑树：以primary key为键，以数据在
// 文件中的行数为值，以两者构成的pair作为红黑树的节点
// 定义从节点得到primary key的规则是获取pair的第一个
// 定义节点的比较规则是上述定义的比较规则
typedef std::_Rb_tree<
    const rbtree_key,
    rbtree_node,
    std::_Select1st<rbtree_node>,
    compare_rule_for_primary_key>
    RedBlackTree;

// 经过封装的红黑树
// 支持primary key相同的数据
class rbtree
{
protected:
    RedBlackTree tree_;

public:
    rbtree() = default;
    ~rbtree() = default;

    // 传入primary key和对应行在文件中的行数，构成一个节点添加到红黑树
    void add_node(const rbtree_key &key, order_of_row_in_file row_order);

    // 传入primary key和对应行在文件中的行数，构成一个节点添加到红黑树
    void add_node(const std::list<primary_key_type> &key, order_of_row_in_file row_order);

    // 寻找primary key等于给定key值的所有值组成的链表，用完需要自行释放空间
    std::list<order_of_row_in_file> *find_values_by_primary_keys(const rbtree_key &key);

    // 传入两个key值，返回所有在两者中间的值组成的链表，用完需要自行释放空间
    // 这里不做索引优化，纯查找位于两个pk之间的值，需保证传入的参数符合primary key格式
    std::list<order_of_row_in_file> *find_values_between_primary_keys(
        const rbtree_key &min, const rbtree_key &max);
};

#endif