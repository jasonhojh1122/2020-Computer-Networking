#ifndef _REQUESTHANDLER_H
#define _REQUESTHANDLER_H

#include <string>
#define MAX_NAME_LEN

enum StatusCode {
    OK,
    NOT_FOUND
};

class RequestHandler {
public:
    RequestHandler(std::string& request, std::string& response);

private:
    std::string request;
    std::string response;
    long request_len;
    char fileName[MAX_NAME_LEN];

    void generateResponse();
    void parseRequest();
    StatusCode readFile();
};

#endif