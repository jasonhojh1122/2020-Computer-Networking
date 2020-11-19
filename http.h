#include <string>
#include <vector>

namespace http {

enum StatusCode {
    STATUS_OK,
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

struct HTTPRequest{
    std::vector<std::string>    parsed_request;
    StatusCode                  status_code;
    HTTPMethod                  http_method;
    FileType                    file_type;
    std::string                 file_name;
};

void        response_thread(int conn_fd, std::string request);
void        parseHttpRequest(std::string& request, HTTPRequest& http_request);
void        parseFileRequest(HTTPRequest& http_request);
FileType    getFileType(std::string& file_name);
void        getFile(std::string file_name, std::string& fileData, FileType& file_type, StatusCode& status_code);
std::string getResponseMessage(StatusCode status_code, FileType file_type, std::string& file_data);
}