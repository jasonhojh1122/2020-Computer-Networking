#include "IO.h"

void IO::sendAll(int conn_fd, std::string& message) {
    char *ptr = message.data();
    int len = message.length();
    while (len > 0) {
        int sent_len;
        sent_len = send(conn_fd, ptr, len, 0);
        ptr += sent_len;
        len -= sent_len;
    }
    return;
}

bool IO::readFile(std::string& file_name, std::ios::openmode openmode, std::string& file_data) {   
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file_data.clear();
        ifs.open(file_name, openmode);
        file_data.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        ifs.close();
        return true;
    }
    catch (std::ifstream::failure& e) {
        #ifdef VERBOSE
        std::cout << "Read file error: " << e.what() << "\n";
        #endif
        return false;
    }
}

bool IO::isUsingParentDirectory(std::string& file_name) {
    std::string tmp;
    std::stringstream ss(file_name);
    while (std::getline(ss, tmp, '/')) {
        if (tmp.compare("..") == 0) {
            return true;
        }
    }
    return false;
}

IO::FileType IO::getFileType(std::string& file_name) {
    std::string tmp;
    std::stringstream ss(file_name);
    while (std::getline(ss, tmp, '.')) {
        continue;
    }
    if (tmp.compare("html") == 0)
        return TYPE_HTML;
    else if (tmp.compare("jpeg") == 0)
        return TYPE_JPEG;
    else if (tmp.compare("jpg") == 0)
        return TYPE_JPG;
    else if (tmp.compare("png") == 0)
        return TYPE_PNG;
    else 
        return TYPE_UNDEFINED;
}

std::ios::openmode IO::getOpenmode(FileType file_type) {
    std::ios::openmode openmode;
    if (file_type == TYPE_HTML) 
        openmode = std::ios::in;
    else
        openmode = std::ios::in & std::ios::binary;
    return openmode;
}