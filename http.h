#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>

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

enum FileType {
    TYPE_HTML,
    TYPE_JPEG,
    TYPE_JPG,
    TYPE_PNG,
    TYPE_UNDEFINED
};

struct HTTPRequest {
    std::vector<std::string>    parsed_request;
    HTTPMethod                  http_method;
    std::string                 file_name;
};

struct HTTPResponse {
    StatusCode                  status_code;
    FileType                    file_type;
    std::string                 file_data;
};

void        response_thread(int conn_fd, std::string request);
void        parseHttpRequest(std::string& request, HTTPRequest& http_request);
FileType    getFileType(std::string& file_name);
bool        readFile(std::string& file_name, std::ios::openmode openmode, std::string& file_data);
void        getHTTPResponse(std::string& file_name, HTTPResponse& http_response);
void        getResponseHeader(HTTPResponse& http_response, std::string& message);
bool        isUsingParentDirectory(std::string& file_name);
}