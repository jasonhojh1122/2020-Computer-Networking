#ifndef _IO_H
#define _IO_H
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>

namespace IO {

enum FileType {
    TYPE_HTML,
    TYPE_JPEG,
    TYPE_JPG,
    TYPE_PNG,
    TYPE_UNDEFINED
};

void sendAll(int conn_fd, std::string& message);

bool readFile(std::string& file_name, std::ios::openmode openmode, std::string& file_data);

bool isUsingParentDirectory(std::string& file_name);

FileType getFileType(std::string& file_name);

std::ios::openmode getOpenmode(FileType file_type);


}

#endif
