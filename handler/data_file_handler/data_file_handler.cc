#include "data_file_handler.h"

// 判断文件是否存在
bool DataFileHandler::file_exist(const string &path)
{
    return access(path.c_str(), 0) == 0;
}

DataFileHandler::DataFileHandler(uint16_t number_of_page_to_cache)
{
    number_of_page_to_cache_ = number_of_page_to_cache;
}

DataFileHandler::~DataFileHandler()
{
    if (file_.is_open())
        file_.close();

    // 删除缓存
    for (auto it = pages_cache_.begin(); it != pages_cache_.end(); it++)
    {
        if (it->second)
            delete[] (char *)(it->second);
    }
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

        // 获取文件大小
        file_.seekg(0, ios::end);
        file_size_ = file_.tellg();
    }
    else // 只读模式
    {
        open_mode_of_file |= ios::in;
        file_.open(file_path.c_str(), (std::ios_base::openmode)open_mode_of_file);

        if (!file_.is_open())
            return error_code::ERROR_FILE_NOT_EXISTS;

        // 获取文件大小
        file_.seekp(0, ios::end);
        file_size_ = file_.tellp();
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

    // 加一位存储\0
    char *char_file_content = new char[buffer_length + 1];

    if (!char_file_content)
        return respond<string>("", error_code::ERROR_MEMORY_ALLOCATION_FAIL);

    file_.read(char_file_content, buffer_length);
    char_file_content[buffer_length] = '\0';

    string file_content = char_file_content;

    delete[] char_file_content;

    return respond<string>(file_content, error_code::SUCCESS);
}

void *DataFileHandler::read_page_to_memory(uint32_t page_order)
{
    // 开始读的地址
    uint64_t start_position = (page_order - 1) * page_size_;

    // 检查页码的合法性
    if (start_position >= file_size_)
    {
        cout << "Warning: demanding page to cache exceeds the file size" << endl;
        return NULL;
    }

    file_.seekg(start_position);

    // 如果剩下的文件内容不够一页4k的大小
    // （取等号意味着刚刚好够读完）
    if (page_order * page_size_ > file_size_)
    {
        void *buffer = new char[file_size_ - start_position];
        file_.read((char *)buffer, file_size_ - start_position);
        return buffer;
    }
    else // 足够读
    {
        void *buffer = new char[file_size_];
        file_.read((char *)buffer, file_size_);
        return buffer;
    }
}

void *DataFileHandler::LRU_get(uint32_t page_order)
{
    // 如果已经存在，则提到最前，返回
    for (auto it = LRU_cache_list.begin(); it != LRU_cache_list.end(); it++)
    {
        if (*it == page_order)
        {
            uint32_t temp = *it;
            LRU_cache_list.erase(it);
            LRU_cache_list.push_front(temp);
            return pages_cache_[temp];
        }
    }

    // 如果不存在，则读取、放到最前、并挤掉最后一个，返回
    void *buffer = read_page_to_memory(page_order);

    // debug
    if (!buffer)
        raise_exception(error_code::ERROR_DEMANDING_PAGE_TO_CACHE_EXCEEDS_THE_FILE_SIZE);

    // 如果已满
    if (LRU_cache_list.size() == number_of_page_to_cache_)
    {
        // 找到最后一项
        uint32_t order_of_page_to_delete = LRU_cache_list.back();
        auto it = pages_cache_.find(order_of_page_to_delete);

        // debug 如果找不到，程序一定有bug
        if (it == pages_cache_.end())
            raise_exception(error_code::ERROR_CACHE_MAP_AND_LRU_INFO_DO_NOT_MATCH);

        // 淘汰掉优先级最低缓存
        pages_cache_.erase(it);
        LRU_cache_list.pop_back();
    }

    // 加入新的
    pages_cache_[page_order] = buffer;
    LRU_cache_list.push_front(page_order);

    // 最后返回
    return buffer;
}

// 把lines读进缓存中，返回指向缓存的指针与可用的行数（防止越界）
// 若创建内存失败，返回空指针
pair<void *, uint32_t> DataFileHandler::read_lines_into_buffer(
    const uint64_t &start_position,
    const int &line_size,
    const int &number_of_line)
{
    if (start_position >= file_size_)
    {
        return pair<void *, int>(NULL, 0);
    }

    uint32_t available_lines;

    // 比较：读完所有内容后的位置 和 文件字节数 的大小关系（取等号意味着刚刚好读完）
    if (start_position + line_size * number_of_line > file_size_)
    {
        // 计算可读行数目
        available_lines = (file_size_ - start_position) / line_size;
    }
    else
    {
        available_lines = line_size;
    }
    uint32_t buffer_size = available_lines * line_size;

    // 分配内存
    void *new_buffer = new char[buffer_size];

    if (!new_buffer)
    {
        cout << "memory allocation failed while reading file" << endl;
        return pair<void *, int>(NULL, 0);
    }
    
    // 计算start_position需要在哪一页开始读（包括边界情况）
    // 如果一页大小为40，start_position=40，那么需要从第二页开始读
    // 如果一页大小为40，start_position=41，那么需要从第二页开始读
    int order_of_first_page = start_position / page_size_ + 1;

    // 计算磁盘读取的结束地址
    int end_position = start_position + buffer_size;

    // debug
    if (end_position > file_size_)
        raise_exception(error_code::ERROR_DEMANDING_LINES_TO_READ_EXCEEDS_THE_FILE_SIZE);

    // 计算最后需要读取到哪一页（包括边界情况）
    // 如果一页大小为40，end_position=40，那么只要读到第一页
    // 如果一页大小为40，end_position=41，那么需要读到第二页
    int order_of_last_page = end_position / page_size_;

    // 若不位于位于边界则需要+1页
    if (end_position % page_size_ != 0)
        order_of_last_page++;

    // 用于内存拷贝，指向还没被赋值的内存段的首位置
    int current_position = 0;

    // 第一页内容需要特殊处理
    void *first_page = LRU_get(order_of_first_page);

    // 计算在第一页内存里面的开始位置
    // 若一页为40，在43开始，那么该值=43%40
    int start_position_in_the_first_page = start_position % page_size_;

    // 是否读取到第一页结尾
    // 取等号时，正好读完第一页
    if (order_of_first_page != order_of_last_page)
    {
        // 读取到结尾
        current_position = page_size_ - start_position_in_the_first_page;
        memcpy(new_buffer, first_page + start_position_in_the_first_page, current_position);
    }
    else
    {
        // 第一页都没读完就结束了或者第一页正好读完，直接return
        memcpy(new_buffer, first_page + start_position_in_the_first_page, buffer_size);
        return pair<void *, int>(new_buffer, available_lines);
    }

    // 处理除了第一页和最后一页以外的所有页
    // 如果一共也就两页，那么不会执行这个循环体
    for (int order_of_current_page = order_of_first_page + 1; order_of_current_page < order_of_last_page; order_of_current_page++)
    {
        // 读取整个page_size
        void *page = LRU_get(order_of_current_page);
        memcpy(new_buffer + current_position, page, page_size_);
        current_position += page_size_;
    }

    // 最后一页也需要特殊处理
    void *last_page = LRU_get(order_of_last_page);
    memcpy(new_buffer + current_position, last_page, end_position % page_size_);

    // 返回内存块与可用的行数
    return pair<void *, int>(new_buffer, available_lines);
}