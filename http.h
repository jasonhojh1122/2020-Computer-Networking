#ifndef _HTTP_H
#define _HTTP_H
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <map>

#include "IO.h"

extern char* ROOT_DIR;

namespace http {

static std::string HTTP_ERROR_FOLDER = "/http_error";

enum StatusCode {
    STATUS_OK,
    STATUS_FORBIDDEN,
    STATUS_NOT_FOUND
};

enum HTTPMethod {
    METHOD_GET,
    METHOD_POST,
    METHOD_UNDEFINED
};

struct HTTPRequest {
    std::vector<std::string>    parsed_request;
    HTTPMethod                  http_method;
    std::string                 file_name;
    std::map<std::string, std::string> post_value;
};

struct HTTPResponse {
    StatusCode                  status_code;
    IO::FileType                file_type;
    std::string                 header;
    std::string                 file_data;
};

void parseHttpRequest(std::string& request, HTTPRequest& http_request);

void parseHTTPMethod(HTTPRequest& http_request);

void parsePostArg(HTTPRequest& http_request);

void getResponseHeader(HTTPResponse& http_response);

}
#endif