#include <iostream>
#include <map>
#include <functional>
#include <sqlite3.h>
#include <time.h>

#include "http.h"

class Web {
typedef void (Web::*MFP)(http::HTTPRequest &, http::HTTPResponse&);
public:
    Web();
    ~Web();
    void render(std::string& packet, http::HTTPResponse& response);

private:
    std::string db_name = "./project.db";
    std::map<std::string, MFP> service;
    sqlite3 *db;
    void serveFile(std::string& file_name, http::HTTPResponse& response);
    void login(http::HTTPRequest& http_request, http::HTTPResponse& response);
    void signup(http::HTTPRequest& http_request, http::HTTPResponse& response);
    void bulletin(http::HTTPRequest& http_request, http::HTTPResponse& response);

    void errorPage(std::string& error_message, http::HTTPResponse& http_response);

    bool getCookieByAccount(std::string& account, std::string& cookie);
    bool insertNewUser(std::string& account, std::string& password);
};