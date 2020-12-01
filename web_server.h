#include <iostream>
#include <map>
#include <functional>

#include "http.h"

typedef std::map<std::string, std::function<void(http::HTTPRequest &, http::HTTPResponse&)>> service_map;

class Web {
public:
    Web();
    ~Web();
    void render(std::string& packet, http::HTTPResponse& response);

private:
    static service_map service;
    static void serveFile(http::HTTPRequest& request, http::HTTPResponse& response);
    static void login(http::HTTPRequest& request, http::HTTPResponse& response);
    static void signup(http::HTTPRequest& request, http::HTTPResponse& response);
    static void bulletin(http::HTTPRequest& request, http::HTTPResponse& response);
};