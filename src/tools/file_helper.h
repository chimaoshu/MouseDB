#ifndef MOUSEDB_SRC_TOOLS_FILE_HELPER_H_
#define MOUSEDB_SRC_TOOLS_FILE_HELPER_H_

#include <string>

// mkdir
#include <sys/stat.h>

namespace FileHelper
{
    // 创建文件夹
    inline int make_dir(const std::string &dir)
    {
        return mkdir(dir.c_str(), S_IRWXU);
    }

    // 文件或目录是否存在
    inline bool dir_or_file_exists(const string &path)
    {
        return access(path.c_str(), 0) == 0;
    }

} // namespace FileHelper

#endif // MOUSEDB_SRC_TOOLS_FILE_HELPER_H_