#include "web_server.h"

service_map Web::service = {
    {"/login",      &Web::login},
    {"/signup",     &Web::signup},
    {"/bulletin",   &Web::bulletin}
};

Web::Web() {}

Web::~Web() {}

void Web::render(std::string& request, http::HTTPResponse& http_response) {
    http::HTTPRequest http_request = {};
    http::parseHttpRequest(request, http_request);

    service_map::iterator it = service.find(http_request.file_name);
    if (it != service.end()) {
        (it->second)(http_request, http_response);
    }
    else {
        Web::serveFile(http_request, http_response);
    }

    http::getResponseHeader(http_response);
}

void Web::login(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    if (http_request.http_method != http::METHOD_POST) {
        http_request.file_name = ROOT_DIR + std::string("/index.html");
        Web::serveFile(http_request, http_response);
    }
    else if (http_request.http_method == http::METHOD_POST) {

    }
}



void Web::signup(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {

}

void Web::bulletin(http::HTTPRequest& http_request, http::HTTPResponse& response) {

}

void Web::serveFile(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {

    std::string file_dir;

    bool forbidden = IO::isUsingParentDirectory(http_request.file_name);
    if (forbidden) {
        file_dir = ROOT_DIR + http::HTTP_ERROR_FOLDER + std::string("/403.html");
        http_response.file_type = IO::TYPE_HTML;
        http_response.status_code = http::STATUS_FORBIDDEN;
        IO::readFile(file_dir, std::ios::in, http_response.file_data);
        return;
    }
    
    IO::FileType file_type;
    file_type = IO::getFileType(http_request.file_name);

    auto openmode = IO::getOpenmode(file_type);

    file_dir = ROOT_DIR + http_request.file_name;

    if (IO::readFile(file_dir, openmode, http_response.file_data)) {
        http_response.file_type = file_type;
        http_response.status_code = http::STATUS_OK;
    }
    else {
        file_dir = ROOT_DIR + http::HTTP_ERROR_FOLDER + std::string("/404.html");
        http_response.file_type = IO::TYPE_HTML;
        http_response.status_code = http::STATUS_NOT_FOUND;
        IO::readFile(file_dir, std::ios::in, http_response.file_data);
    }
    return;
    
}