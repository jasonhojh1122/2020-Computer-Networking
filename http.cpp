#include "http.h"

void http::parseHttpRequest(std::string& request, HTTPRequest& http_request) {

    std::string tmp;
    std::stringstream header(request);
    while (std::getline(header, tmp, '\n')) {
        http_request.parsed_request.push_back(tmp);
    }

    
}

void http::parseHTTPMethod(HTTPRequest& http_request) {
    std::stringstream ss(http_request.parsed_request[0]);
    std::string tmp;

    std::getline(ss, tmp, ' ');

    if (tmp.compare("GET") == 0)
        http_request.http_method = METHOD_GET;
    else if (tmp.compare("POST") == 0)
        http_request.http_method = METHOD_POST;
    else 
        http_request.http_method = METHOD_UNDEFINED;

    std::getline(ss, http_request.file_name, ' ');
}

void http::parsePostArg(HTTPRequest& http_request) {
    
}

void http::getResponseHeader(HTTPResponse& http_response) {
    std::stringstream ss;
    ss << "HTTP/1.1 ";
    if (http_response.status_code == STATUS_OK)
        ss << "200 OK\r\n";
    else if (http_response.status_code == STATUS_NOT_FOUND)
        ss << "404 NOT FOUND\r\n";
    
    ss << "Content-Type: ";
    switch (http_response.file_type) {
        case IO::TYPE_HTML:
            ss << "text/html\r\n";
            break;
        case IO::TYPE_JPEG:
            ss << "image/jpeg\r\n";
            break;
        case IO::TYPE_JPG:
            ss << "image/jpg\r\n";
            break;
        case IO::TYPE_PNG:
            ss << "image/png\r\n";
            break;
        case IO::TYPE_UNDEFINED:
            ss << "application/x-binary\r\n";
    }

    ss << "Content-Length: " << http_response.file_data.length() << "\r\n";
    
    ss << "Connection: close\r\n";
    
    ss << "\r\n";

    http_response.header = ss.str();
}