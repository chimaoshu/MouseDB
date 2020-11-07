#ifndef MOUSEDB_HANDLER_DATA_FILE_HANDLER_H_
#define MOUSEDB_HANDLER_DATA_FILE_HANDLER_H_

#include<string>
#include<fstream>

using namespace std;

class DataFileHandler
{
private:
    fstream file_;
    
    bool file_exist(const string &path);

public:
    string path_;
    bool writable_;
    bool readable_;
    bool appendable_;
    bool truncate_;

    ~DataFileHandler();

    bool is_open();
    void open(const string &file_path, bool write = 1, bool append = 0, bool truncate = 0, bool read = 1);
    void append(const string &string_to_write);
    string read_line();
    string read_all();
};
#endif // MOUSEDB_HANDLER_DATA_FILE_HANDLER_H_