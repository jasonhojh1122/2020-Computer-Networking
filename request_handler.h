#ifndef _REQUESTHANDLER_H
#define _REQUESTHANDLER_H

#include <string>
#include <vector>

#define MAX_NAME_LEN

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

class RequestHandler {
public:
    RequestHandler(std::string root);
    std::string getResponse(std::string& requset);

private:
    std::string root_dir;

    void        parseRequest(std::string& request, std::vector<std::string>& parsed_request, HTTPMethod& method, std::string& file_name);
    FileType    getFileType(std::string& file_name);
    void        getFile(std::string file_name, std::string& fileData, FileType& file_type, StatusCode& status_code);
    std::string getResponseMessage(StatusCode status_code, FileType file_type, std::string& file_data);
};

#endif