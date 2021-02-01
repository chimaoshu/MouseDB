#ifndef MOUSEDB_SRC_TOOLS_TOOLS_H_
#define MOUSEDB_SRC_TOOLS_TOOLS_H_

#include <string>

// mkdir
#include <sys/stat.h>

// access
#include <unistd.h>

namespace Tools {
    
// 创建文件夹
inline int make_dir(const std::string &dir) {
  return mkdir(dir.c_str(), S_IRWXU);
}

// 文件或目录是否存在
inline bool dir_or_file_exists(const std::string &path) {
  return access(path.c_str(), 0) == 0;
}

// 获取字符串时间戳
inline std::string get_str_timestamp() { return std::to_string(time(NULL)); }

// 获取数值时间戳
inline time_t get_timestamp() { return time(NULL); }

} // namespace Tools

#endif // MOUSEDB_SRC_TOOLS_TOOLS_H_