#include "request_handler.h"

RequestHandler::RequestHandler(std::string& request, std::string& response)
: request(request), response(response) {
    generateResponse();
}

void RequestHandler::generateResponse() {
    std::string fileName = getFileName();
    StatusCode status = readFile();
}

void RequestHandler::parseRequest() {

}