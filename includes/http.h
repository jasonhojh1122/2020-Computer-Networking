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

static std::string HTTP_ERROR_FOLDER = "http_error/";

enum StatusCode {
    STATUS_OK,
    STATUS_FORBIDDEN,
    STATUS_SEE_OTHER,
    STATUS_FOUND,
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
    std::vector<std::string>    services;
    std::map<std::string, std::string> post_value;
    std::map<std::string, std::string> cookie;
};

struct HTTPResponse {
    StatusCode                  status_code;
    IO::FileType                file_type;
    std::string                 file_data;
    std::string                 header;
    std::string                 see_other_location;
    bool                        logout = false;
    std::map<std::string, std::string> cookie;
};

void parseHttpRequest(std::string& request, HTTPRequest& http_request);

void parseFirstRow(HTTPRequest& http_request);

void parsePostArg(HTTPRequest& http_request);

void parseCookie(HTTPRequest& http_request);

void getResponseHeader(HTTPResponse& http_response);

std::string intToHTTPDayName(int day);

std::string intToHTTPMonthName(int month);

std::string getHTTPDate(int UTC, int offset_hour);

}
#endif