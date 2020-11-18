#include "request_handler.h"
#include <sstream>
#include <fstream>
#include <iostream>

RequestHandler::RequestHandler(std::string root_dir) : root_dir(root_dir) {}

std::string RequestHandler::getResponse(std::string& request) {
    std::vector<std::string>    parsed_request;
    HTTPMethod                  method;
    std::string                 file_name;
    parseRequest(request, parsed_request, method, file_name);

    if (method != METHOD_GET)
        file_name = "405.html";


    FileType file_type;
    StatusCode status_code;
    std::string file_data;
    getFile(file_name, file_data, file_type, status_code);

    return getResponseMessage(status_code, file_type, file_data);
}

void RequestHandler::parseRequest(  std::string& request, 
                                    std::vector<std::string>& parsed_request, 
                                    HTTPMethod& method, 
                                    std::string& file_name) {
    std::string tmp;
    std::stringstream header(request);
    while (std::getline(header, tmp, '\n')) {
        parsed_request.push_back(tmp);
    }

    std::stringstream ss(parsed_request[0]);

    std::getline(ss, tmp, ' ');

    if (tmp.compare("GET") == 0)
        method = METHOD_GET;
    else if (tmp.compare("POST") == 0)
        method = METHOD_POST;
    else 
        method = METHOD_UNDEFINED;

    std::getline(ss, file_name, ' ');
}

FileType RequestHandler::getFileType(std::string& file_name) {
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

void RequestHandler::getFile(   std::string     file_name, 
                                std::string&    file_data, 
                                FileType&       file_type, 
                                StatusCode&     status_code) {
    file_type = getFileType(file_name);
    std::string file_dir = root_dir + file_name;
    std::ios::openmode openmode;
    if (file_type == TYPE_HTML)
        openmode = std::ios::in;
    else
        openmode = std::ios::in & std::ios::binary;
    std::ifstream ifs(file_dir, openmode);
    
    if (ifs.is_open()) {
        status_code = STATUS_OK;
    }
    else {
        status_code = STATUS_NOT_FOUND;
        file_type = TYPE_HTML;
        openmode = std::ios::in;
        file_dir = root_dir + "/404.html";
        ifs.open(file_dir, openmode);
    }
    file_data.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() );
}

std::string RequestHandler::getResponseMessage(StatusCode status_code, FileType file_type, std::string& file_data) {
    std::stringstream ss;
    ss << "HTTP/1.1 ";
    if (status_code == STATUS_OK)
        ss << "200 OK\r\n";
    else if (status_code == STATUS_NOT_FOUND)
        ss << "404 NOT FOUND\r\n";
    
    ss << "Content-Type: ";
    switch (file_type) {
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

    ss << "Content-Length: " << file_data.length() << "\r\n";
    
    ss << "\r\n";

    ss << file_data << "\r\n";

    return ss.str();
}