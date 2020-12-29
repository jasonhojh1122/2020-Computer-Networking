
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include <sqlite3.h>
#include <curl/curl.h>

#include "http.h"

#define BUFFER_SIZE 1024

namespace web {

void replaceString(std::string& original, std::string& oldString, std::string& newString);

void stringReplaceAll(std::string& origin, std::string toReplace, std::string newStr);

void replacePlusToSpace(std::string& s);

bool isDigits(std::string& s);

void url_escape(std::string encoded, std::string& decoded);

void response_thread(int conn_fd);

class WebServer {
typedef void (WebServer::*MFP)(http::HTTPRequest &, http::HTTPResponse&);
public:
    WebServer();
    ~WebServer();
    void render(std::string& packet, http::HTTPResponse& response);

private:
    std::string db_name = "./project.db";
    std::map<std::string, MFP> service;
    sqlite3 *db;
    int db_timeout = 1000;

    void serveFile(std::string& file_name, http::HTTPResponse& http_response);

    void login(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void login_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void login_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response);

    void logout(http::HTTPRequest& http_request, http::HTTPResponse& http_response);

    void signup(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void signup_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void signup_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    bool insertNewUser(std::string& account, std::string& password);

    void bulletin(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void bulletin_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void bulletin_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    bool getBulletinContentHTML(std::string& content, int limit, int offset, bool& is_end);
    bool insertNewMsgToBulletin(std::string& message, int user_id);

    void errorPage(std::string& error_message, http::HTTPResponse& http_response);

    bool getCookieByAccount(std::string account, unsigned int& cookie);
    bool getIDByAccount(std::string account, int& id);
    bool getIDByCookie(std::string& cookie, int& id);
    bool getAccountByID(int id, std::string& account);
    bool updateCookieByAccount(std::string account);
};

}