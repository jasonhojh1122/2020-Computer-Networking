#include "http.h"

extern const char* ROOT_DIR;

void http::parseHttpRequest(std::string& request, HTTPRequest& http_request) {

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

http::FileType http::getFileType(std::string& file_name) {
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

bool http::isUsingParentDirectory(std::string& file_name) {
    std::string tmp;
    std::stringstream ss(file_name);
    while (std::getline(ss, tmp, '/')) {
        if (tmp.compare("..") == 0) {
            return true;
        }
    }
    return false;
}

bool http::readFile(std::string& file_name, std::ios::openmode openmode, std::string& file_data) {   
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file_data.clear();
        ifs.open(file_name, openmode);
        file_data.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        ifs.close();
        return true;
    }
    catch (std::ifstream::failure e) {
        return false;
    }
}

void http::getHTTPResponse(std::string& file_name, HTTPResponse& http_response) {

    std::string file_dir;

    bool forbidden = isUsingParentDirectory(file_name);
    if (forbidden) {
        file_dir = ROOT_DIR + HTTP_ERROR_FOLDER + std::string("/403.html");
        http_response.file_type = TYPE_HTML;
        http_response.status_code = STATUS_FORBIDDEN;
        readFile(file_dir, std::ios::in, http_response.file_data);
        return;
    }
    else {
        FileType file_type;
        file_type = getFileType(file_name);
        std::ios::openmode openmode;
        if (file_type == TYPE_HTML) {
            openmode = std::ios::in;
        }
        else {
            openmode = std::ios::in & std::ios::binary;
        }

        file_dir = ROOT_DIR + file_name;

        if (readFile(file_dir, openmode, http_response.file_data)) {
            http_response.file_type = file_type;
            http_response.status_code = STATUS_OK;
        }
        else {
            file_dir = ROOT_DIR + HTTP_ERROR_FOLDER + std::string("/404.html");
            http_response.file_type = TYPE_HTML;
            http_response.status_code = STATUS_NOT_FOUND;
            readFile(file_dir, std::ios::in, http_response.file_data);
        }
        return;
    }
}

void http::getResponseHeader(HTTPResponse& http_response, std::string& message) {
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
    
    ss << "Connection: close\r\n";
    
    ss << "\r\n";

    message = ss.str();
}