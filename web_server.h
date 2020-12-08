#include <iostream>
#include <map>
#include <functional>
#include <sqlite3.h>
#include <time.h>
#include <curl/curl.h>

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
    int db_timeout = 1000;

    void serveFile(std::string& file_name, http::HTTPResponse& http_response);

    void login(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void login_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response);
    void login_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response);

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