#ifndef MOUSEDB_SRC_TOOLS_RBTREE_H_
#define MOUSEDB_SRC_TOOLS_RBTREE_H_

#include <map>
#include <vector>
#include <list>
#include <iostream>

#include "src/tools/type.h"

struct compare_rule_for_primary_key;

// type of primary key, it must use int32, just for convenience :-)
typedef int32_t primary_key_type;

// we assume that the largest row number in table is 2^32
// so we use int32 as the row order in table file.
typedef uint32_t order_of_row_in_file;

// define the key of a rbtree (primary key of the table)
// the size() of the vector equals to the number of primary key
// The primary key with a smaller index has a higher priority.
typedef std::vector<primary_key_type> rbtree_key;

// define the value of a rbtree, it is a pair that contains
// primary keys and the line order of that row in file.
typedef std::pair<rbtree_key, order_of_row_in_file> rbtree_value;
typedef rbtree_value rbtree_node;

// define the red_black_tree
typedef std::_Rb_tree<
    const rbtree_key,
    rbtree_node,
    std::_Select1st<rbtree_node>,
    compare_rule_for_primary_key>
    RedBlackTree;

// compare rule for primary keys
// this struct tells which rbtree_key is the less
struct compare_rule_for_primary_key
{
    inline bool
    operator()(const rbtree_key &x, const rbtree_key &y) const;
};

// rbtree for hot data
class rbtree
{
private:
    RedBlackTree tree_;

public:
    rbtree() = default;
    ~rbtree() = default;

    // add by primary key(vector) and row_order
    void add_node(const rbtree_key &key, order_of_row_in_file row_order);

    // add by primary key(list) and row_order
    void add_node(const std::list<primary_key_type> &key, order_of_row_in_file row_order);

    // delete the first node(the minimum one) that match the given key
    void delete_node_by_key(const rbtree_key &key);

    // delete all nodes with primary keys equals to the given key
    void delete_nodes_by_key(const rbtree_key &key);

    // return the pointer pointing to the values list
    std::list<order_of_row_in_file> *find_values_by_primary_keys(const rbtree_key &key);

    // index optimization isn't excuted here, it just find values between two primary key.
    std::list<order_of_row_in_file> *find_values_between_primary_keys(
        const rbtree_key &min, const rbtree_key &max);
    
};

#endif