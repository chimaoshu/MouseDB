#include "data_file_handler.h"

// 判断文件是否存在
bool DataFileHandler::file_exist(const string &path)
{
    return access(path.c_str(), 0) == 0;
}

DataFileHandler::~DataFileHandler()
{
    if (file_.is_open())
        file_.close();
}

void DataFileHandler::close()
{
    if (file_.is_open())
        file_.close();
}

string DataFileHandler::get_path() { return path_; }

status_code DataFileHandler::open(const string &file_path, bool write, bool append, bool truncate, bool read)
{
    // 根据传入参数设置open_mode
    ios::open_mode open_mode_of_file = ios::binary;

    if (write)
    {
        // 可写
        open_mode_of_file |= ios::out;

        // 追加
        if (append)
            open_mode_of_file |= ios::app;

        // 可读
        if (read)
            open_mode_of_file |= ios::in;

        // 若文件不存在，跳过truncate参数，否则会出错
        if (truncate && file_exist(file_path))
        {
            open_mode_of_file |= ios::trunc;
            truncate = false;
        }

        file_.open(file_path.c_str(), (std::ios_base::openmode)open_mode_of_file);

        if (!file_.is_open())
            return error_code::ERROR_WRONG_OPEN_MODE;
    }
    else // 只读模式
    {
        open_mode_of_file |= ios::in;
        file_.open(file_path.c_str(), (std::ios_base::openmode)open_mode_of_file);

        if (!file_.is_open())
            return error_code::ERROR_FILE_NOT_EXISTS;
    }

    // 存储文件open_mode
    path_ = file_path;
    readable_ = read;
    appendable_ = append;
    writable_ = write;
    truncate_ = truncate;

    return error_code::SUCCESS;
}

bool DataFileHandler::is_open() { return file_.is_open(); }

// 文件尾追加字符串
status_code DataFileHandler::append(const string &string_to_write)
{
    if (!writable_)
        return error_code::ERROR_NOT_APPENDABLE;

    if (!appendable_)
        file_.seekp(0, ios::end);

    file_.write(string_to_write.c_str(), string_to_write.size());

    return error_code::SUCCESS;
}

// 文件尾追加二进制数据
status_code DataFileHandler::append(void *buffer, int buffer_size)
{
    if (!writable_)
        return error_code::ERROR_NOT_APPENDABLE;

    if (!appendable_)
        file_.seekp(0, ios::end);

    file_.write((char *)buffer, buffer_size);

    return error_code::SUCCESS;
}

// 读字符串
respond<string> DataFileHandler::read_line()
{
    if (writable_ && !readable_)
        return respond<string>("", error_code::ERROR_NOT_READABLE);

    string next_line;
    getline(file_, next_line);

    return respond<string>(next_line, error_code::SUCCESS);
}

// 读字符串
respond<string> DataFileHandler::read_all()
{
    if (writable_ && !readable_)
        return respond<string>("", error_code::ERROR_NOT_READABLE);

    // 移动至末尾
    file_.seekg(0, ios::end);

    // 获取位置，位置即文件长度
    int buffer_length = file_.tellg();

    // 回到开头
    file_.seekg(0, ios::beg);

    char *char_file_content = new char[buffer_length];

    if (!char_file_content)
        return respond<string>("", error_code::ERROR_MEMORY_ALLOCATION_FAIL);

    file_.read(char_file_content, buffer_length);

    string file_content = char_file_content;

    delete[] char_file_content;

    return respond<string>(file_content, error_code::SUCCESS);
}

// 把lines读进缓存中，返回指向缓存的指针与可用的行数（防止越界）
// 若创建内存失败，返回空指针
pair<void *, uint64_t> DataFileHandler::read_lines_into_buffer(
    const int &initial_adress,
    const int &line_size,
    const int &number_of_line)
{
    file_.seekg(0, ios::end);
    uint64_t file_size = file_.tellg();

    if (initial_adress > file_size)
    {
        return pair<void *, int>(NULL, 0);
    }

    uint64_t available_lines;

    // 计算可读行数目
    if (initial_adress + line_size * number_of_line > file_size)
    {
        available_lines = (file_size - initial_adress) / line_size;
    }
    else
    {
        available_lines = line_size;
    }
    uint64_t buffer_size = available_lines * line_size;

    // 分配内存
    void *p = new char[buffer_size];

    if (!p)
    {
        cout << "memory allocation failed while reading file" << endl;
        return pair<void *, int>(NULL, 0);
    }

    file_.seekg(initial_adress);
    file_.read((char *)p, line_size * number_of_line);

    return pair<void *, int>(p, available_lines);
    ;
}