#include "http.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

extern const char* root_dir;

namespace http {

void parseHttpRequest(std::string& request, HTTPRequest& http_request) {

    std::string tmp;
    std::stringstream header(request);
    while (std::getline(header, tmp, '\n')) {
        http_request.parsed_request.push_back(tmp);
    }

    std::stringstream ss(http_request.parsed_request[0]);

    std::getline(ss, tmp, ' ');

    if (tmp.compare("GET") == 0)
        http_request.http_method = METHOD_GET;
    else if (tmp.compare("POST") == 0)
        http_request.http_method = METHOD_POST;
    else 
        http_request.http_method = METHOD_UNDEFINED;

    std::getline(ss, http_request.file_name, ' ');
}

FileType getFileType(std::string& file_name) {
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
    else return TYPE_UNDEFINED;
}

void getFile(std::string& file_name, HTTPResponse& http_response) {
    FileType file_type = getFileType(file_name);

    std::ios::openmode openmode;
    if (file_type == TYPE_HTML)
        openmode = std::ios::in;
    else
        openmode = std::ios::in & std::ios::binary;

    std::string file_dir = root_dir + file_name;
    std::ifstream ifs(file_dir, openmode);
    
    if (ifs.is_open()) {
        http_response.status_code = STATUS_OK;
    }
    else {
        http_response.status_code = STATUS_NOT_FOUND;
        file_type = TYPE_HTML;
        openmode = std::ios::in;
        file_dir = root_dir + std::string("/404.html");
        ifs.open(file_dir, openmode);
    }

    http_response.file_type = file_type;
    http_response.file_data.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() );
}

void getResponseMessage(HTTPResponse& http_response, std::string& message) {
    std::stringstream ss;
    ss << "HTTP/1.1 ";
    if (http_response.status_code == STATUS_OK)
        ss << "200 OK\r\n";
    else if (http_response.status_code == STATUS_NOT_FOUND)
        ss << "404 NOT FOUND\r\n";
    
    ss << "Content-Type: ";
    switch (http_response.file_type) {
        case TYPE_HTML:
            ss << "text/html\r\n";
            break;
        case TYPE_JPEG:
            ss << "image/jpeg\r\n";
            break;
        case TYPE_JPG:
            ss << "image/jpg\r\n";
            break;
        case TYPE_PNG:
            ss << "image/png\r\n";
            break;
        case TYPE_UNDEFINED:
            ss << "application/x-binary\r\n";
    }

    ss << "Content-Length: " << http_response.file_data.length() << "\r\n";
    
    ss << "\r\n";

    ss << http_response.file_data << "\r\n";

    message = ss.str();
}

}