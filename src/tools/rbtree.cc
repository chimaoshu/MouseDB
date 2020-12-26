#include "src/tools/rbtree.h"

using namespace std;

bool compare_rule_for_primary_key::operator()(const rbtree_key &x, const rbtree_key &y) const
{
    // debug
    assert(x.size() == y.size());

    for (int i = 0; i < x.size(); i++)
    {
        if (x[i] < y[i])
        {
            return true;
        }
        else if (x[i] > y[i])
        {
            return false;
        }
    }
}

// add by primary key(vector) and row_order
void rbtree::add_node(const rbtree_key &key, order_of_row_in_file row_order)
{
    tree_._M_insert_equal(rbtree_value(key, row_order));
}

// add by primary key(list) and row_order
void rbtree::add_node(const list<primary_key_type> &key, order_of_row_in_file row_order)
{
    vector<primary_key_type> temp;
    temp.reserve(key.size());

    for (auto it = key.begin(); it != key.end(); it++)
    {
        temp.push_back(*it);
    }

    tree_._M_insert_equal(make_pair(temp, row_order));
}

// delete the first node(the minimum one) that match the given key
void rbtree::delete_node_by_key(const rbtree_key &key)
{
    auto it = tree_.find(key);

    if (it != tree_.end())
        tree_.erase(it);
}

// delete all nodes with primary keys equals to the given parameter
void rbtree::delete_nodes_by_key(const rbtree_key &key)
{
    auto res = tree_.equal_range(key);

    for (auto it = res.first; it != res.second; it++)
        tree_.erase(it);
}

// return the pointer pointing to the values list
list<order_of_row_in_file> *rbtree::find_values_by_primary_keys(const rbtree_key &key)
{
    list<order_of_row_in_file> *values = new list<order_of_row_in_file>;

    auto res = tree_.equal_range(key);
    for (auto it = res.first; it != res.second; it++)
    {
        values->push_back((*it).second);
    }

    return values;
}

// 这里不做索引优化，纯查找位于两个pk之间的值，需保证传入的参数符合primary key格式
list<order_of_row_in_file> *rbtree::find_values_between_primary_keys(
    const rbtree_key &min, const rbtree_key &max)
{
    // debug
    assert(max.size() == min.size());

    list<order_of_row_in_file> *values = new list<order_of_row_in_file>;

    auto begin_it = tree_.lower_bound(min);
    auto end_it = tree_.upper_bound(max);

    for (; begin_it != end_it; begin_it++)
    {
        values->push_back((*begin_it).second);
    }

    return values;
}
