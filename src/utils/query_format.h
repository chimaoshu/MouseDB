#ifndef _MOUSEDB_SRC_TOOLS_QUERY_FORMAT_H_
#define _MOUSEDB_SRC_TOOLS_QUERY_FORMAT_H_

#include <vector>

// 单独一个字段的查询格式
struct QueryFormatOfSingleField {

  // 上限
  int upper_limit;

  // 是否包含等于上限的条件
  bool equal_to_the_upper_limit;

  // 下限
  int lower_limit;

  // 是否包含等于下限的条件
  bool equal_to_the_lower_limit;
};

// 查询格式：比如一个表格有3个字段，那么就用一个
// size为3的vector<QueryFormatOfSingleField>
// 来表示查询条件。由于长度是固定的，因此可以使
// 用vector来完成。
typedef std::vector<QueryFormatOfSingleField> QueryFormatOfFields;

// limit offset

#endif // _MOUSEDB_SRC_TOOLS_QUERY_FORMAT_H_