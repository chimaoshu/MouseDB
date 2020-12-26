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

void rbtree::add_node(const rbtree_key &key, order_of_row_in_file row_order)
{
    tree_._M_insert_equal(rbtree_value(key, row_order));
}

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
