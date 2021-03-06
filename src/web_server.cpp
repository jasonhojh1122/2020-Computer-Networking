#include "web_server.h"

void web::replaceString(std::string& original, std::string& oldString, std::string& newString) {
    auto pos = original.find(oldString);
    if (pos != std::string::npos)
        original.replace(pos, oldString.length(), newString);
}

void web::stringReplaceAll(std::string& origin, std::string toReplace, std::string newStr) {
    auto pos = origin.find(toReplace);
    while (pos != std::string::npos) {
        origin.replace(pos, toReplace.length(), newStr);
        pos = origin.find(toReplace);
    }
}

void web::replacePlusToSpace(std::string& s) {
    auto i = s.begin();
    auto n = s.end();
    for (; i != n; i++) 
        if (*i == '+')
            *i = ' ';
}

bool web::isDigits(std::string& s) {
    std::string::size_type i = 0;
    auto n = s.size();
    for (; i < n; i++)
        if (s[i] < '0' || s[i] > '9')
            return false;
    return true;
}

void web::url_escape(std::string encoded, std::string& decoded) {
    CURL *curl = NULL;
    curl = curl_easy_init();
    int outLength;
    char *decoded_c = curl_easy_unescape(curl, encoded.c_str(), encoded.length(), &outLength);
    decoded.assign(decoded_c, outLength);
    curl_free(decoded_c);
    curl_easy_cleanup(curl);
    return;
}

void web::response_thread(int conn_fd) {
    
    int read_len;
    char* buffer = new char[BUFFER_SIZE];
    read_len = recv(conn_fd , buffer, BUFFER_SIZE, 0);
    if (read_len == 0) {
        close(conn_fd);
        return;
    }

    std::string request;
    request.clear();
    request.assign(buffer, read_len);

    
    std::cout << "----------New Request----------\n";
    std::cout << request << '\n';
    

    http::HTTPResponse http_response = {};
    
    web::WebServer web_server = {};
    web_server.render(request, http_response);

    std::cout << "Response\n";
    std::cout << http_response.header << '\n';

    IO::sendAll(conn_fd, http_response.header);
    IO::sendAll(conn_fd, http_response.file_data);
    close(conn_fd);
    delete []buffer;
}

web::WebServer::WebServer() {
    int err;
    err = sqlite3_open(db_name.c_str(), &db);
    if (err) {
        std::runtime_error("Failed to open database.");
        #ifdef VERBOSE
        std::cerr << sqlite3_errmsg(db);
        #endif
        exit(EXIT_FAILURE);
    }
    service.insert({"login", &WebServer::login});
    service.insert({"logout", &WebServer::logout});
    service.insert({"signup", &WebServer::signup});
    service.insert({"bulletin", &WebServer::bulletin});
}

web::WebServer::~WebServer() {
    sqlite3_close(db);
}

void web::WebServer::render(std::string& request, http::HTTPResponse& http_response) {
    http::HTTPRequest http_request = {};
    http::parseHttpRequest(request, http_request);

    if (http_request.services.size() == 0) {
        std::string file_name = http::HTTP_ERROR_FOLDER + "404.html";
        serveFile(file_name, http_response);
        http_response.status_code = http::STATUS_NOT_FOUND;
        http::getResponseHeader(http_response);
        return;
    }

    auto it = service.find(http_request.services[0]);
    if (it != service.end()) {
        (this->*(it->second))(http_request, http_response);
    }
    else {
        std::string file_name = http_request.services[0];
        auto it = http_request.services.begin();
        it += 1;
        for (; it != http_request.services.end(); it++) {
            file_name += '/';
            file_name += *it;
        }
        serveFile(file_name, http_response);
    }

    http::getResponseHeader(http_response);

    #ifdef VERBOSE
    std::cerr << "---------- Response ----------\n";
    std::cerr << http_response.header << '\n';
    #endif
}

void web::WebServer::login(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    if (http_request.http_method == http::METHOD_GET) 
        login_get(http_request, http_response);
    else if (http_request.http_method == http::METHOD_POST)
        login_post(http_request, http_response);
}

void web::WebServer::login_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {

    auto it = http_request.cookie.find("uuid");

    int uid;
    if (it != http_request.cookie.end() && isDigits(it->second) && getIDByCookie(it->second, uid)) {
        http_response.status_code = http::STATUS_SEE_OTHER;
        http_response.see_other_location = "index.html";
        return;
    }
    std::string file_name = "login.html";
    serveFile(file_name, http_response);
}

void web::WebServer::login_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    auto account_it = http_request.post_value.find("account");
    auto password_it = http_request.post_value.find("password");

    if (account_it == http_request.post_value.end() || 
        password_it == http_request.post_value.end()) {
        std::string error_message = "Wrong arguments in post request";
        errorPage(error_message, http_response);
        return;
    }

    const char *sql = "SELECT password FROM user WHERE account = ? ;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::string file_name = http::HTTP_ERROR_FOLDER + std::string("500.html");
        serveFile(file_name, http_response);
        return;
    }

    sqlite3_bind_text(stmt, 1, account_it->second.c_str(), account_it->second.length(), NULL);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        std::string error_message = "Account doesn't exist";
        errorPage(error_message, http_response);
        return;
    }
    else {
        std::string password = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        sqlite3_finalize(stmt);

        if (password_it->second.compare(password) == 0) {
            http_response.status_code = http::STATUS_SEE_OTHER;
            http_response.see_other_location = "index.html";
            
            unsigned int cookie;
            if (updateCookieByAccount(account_it->second))
                if (getCookieByAccount(account_it->second, cookie))
                    http_response.cookie.insert({"uuid", std::to_string(cookie)});
        }
        else {
            std::string error_message = "Password doesn't match";
            errorPage(error_message, http_response);
            return;
        }
    }
}

void web::WebServer::logout(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    http_response.logout = true;
    http_response.status_code = http::STATUS_SEE_OTHER;
    http_response.cookie.insert({"uuid", "0"});
    http_response.see_other_location = "index.html";
    return;
}

void web::WebServer::signup(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    if (http_request.http_method == http::METHOD_GET)
        signup_get(http_request, http_response);
    else if (http_request.http_method == http::METHOD_POST)
        signup_post(http_request, http_response);
}

void web::WebServer::signup_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    auto it = http_request.cookie.find("uuid");
    int uid;
    if (it != http_request.cookie.end() && isDigits(it->second) && getIDByCookie(it->second, uid)) {
        http_response.status_code = http::STATUS_SEE_OTHER;
        http_response.see_other_location = "index.html";
        return;
    }
    std::string file_name = "signup.html";
    serveFile(file_name, http_response);
}

void web::WebServer::signup_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    auto account_it = http_request.post_value.find("account");
    auto password_it = http_request.post_value.find("password");

    if (account_it == http_request.post_value.end() || 
        password_it == http_request.post_value.end()) {
        std::string error_message = "Wrong arguments in post request";
        errorPage(error_message, http_response);
        return;
    }

    const char *sql = "SELECT id FROM user WHERE account = ? ;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::string file_name = http::HTTP_ERROR_FOLDER + std::string("500.html");
        serveFile(file_name, http_response);
        return;
    }
    sqlite3_bind_text(stmt, 1, account_it->second.c_str(), account_it->second.length(), NULL);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string error_message = "Account already exists";
        errorPage(error_message, http_response);
        return;
    }
    sqlite3_finalize(stmt);

    if (insertNewUser(account_it->second, password_it->second)) {
        http_response.status_code = http::STATUS_SEE_OTHER;
        http_response.see_other_location = "signup_success.html";

        unsigned int cookie;
        if (getCookieByAccount(account_it->second, cookie))
            http_response.cookie.insert({"uuid", std::to_string(cookie)});
    }
    else {
        std::string file_name = http::HTTP_ERROR_FOLDER + std::string("500.html");
        serveFile(file_name, http_response);
    }
}

void web::WebServer::bulletin(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    if (http_request.http_method == http::METHOD_GET)
        bulletin_get(http_request, http_response);
    else if (http_request.http_method == http::METHOD_POST)
        bulletin_post(http_request, http_response);
}

void web::WebServer::bulletin_get(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    int page;
    if (http_request.services.size() == 1) {
        page = 1;
    }
    else {
        if (isDigits(http_request.services[1])) {
            page = std::stoi(http_request.services[1]);
        }
        else {
            std::string file_name = http::HTTP_ERROR_FOLDER + "404.html";
            serveFile(file_name, http_response);
        }
    }   

    std::string post_form;
    auto it = http_request.cookie.find("uuid");
    if (it != http_request.cookie.end()) {
        post_form = "<form method='POST'>"
                        "<label for='message'>Message:</label><br>"
                        "<textarea id='message' name='message' rows='5' cols='50'></textarea><br>"
                        "<input type='submit' value='Submit'>"
                     "</form>";
    }
    else {
        post_form = "<p>Please login to post on the bulletin</p>"
                    "<a href='/login'>Click me to login.</a>";
    }

    std::string content;
    bool is_end;
    if (getBulletinContentHTML(content, 10, (page-1) * 10, is_end) == false) {
        std::string error_message = "Can't retrieve bulletin content.";
        errorPage(error_message, http_response);
        return;
    }
    std::string file_name = "bulletin.html";
    serveFile(file_name, http_response);

    std::stringstream ss;
    if (is_end) {
        if (page > 1)
            ss << "<a href='/bulletin/" << page-1 << "'>Prev</a><br>";
    }
    else {
        if (page > 1) {
            ss << "<a href='/bulletin/" << page-1 << "'>Prev</a>";
        }
        ss << "<a href='/bulletin/" << page+1 << "'>Next</a><br>";
    }
    
    std::string page_button = ss.str();

    std::string toReplace = "{Form}";
    replaceString(http_response.file_data, toReplace, post_form);
    toReplace = "{Content}";
    replaceString(http_response.file_data, toReplace, content);
    toReplace = "{Page_Button}";
    replaceString(http_response.file_data, toReplace, page_button);

}

bool web::WebServer::getBulletinContentHTML(std::string& content, int limit, int offset, bool& is_end) {
    sqlite3_stmt *stmt;
    std::stringstream ss;
    const char *sql = "SELECT b.message, u.account FROM bulletin as b, user as u "
                      "WHERE b.user_id = u.id ORDER BY b.id DESC limit ? offset ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "getBulletinContentHTML prepare: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }

    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, offset);

    sqlite3_busy_timeout(db, db_timeout);
    int cnt = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string msg = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        std::string account = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        
        ss <<   "<p>"
                "User: " << account << "<br>"
           <<   "Message: <br>"
           <<   msg
           <<   "<br><p><hr>";
        cnt++;
    }
    sqlite3_finalize(stmt);
    if (cnt < limit) is_end = true;
    else is_end = false;
    content = ss.str();
    return true;
}

void web::WebServer::bulletin_post(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    auto msg_it = http_request.post_value.find("message");
    auto cookie_it = http_request.cookie.find("uuid");
    
    if (msg_it == http_request.post_value.end() ) {
        std::string error_message = "Wrong arguments in post request";
        errorPage(error_message, http_response);
        return;
    }
    if (cookie_it == http_request.cookie.end()) {
        std::string error_message = "Please login first";
        errorPage(error_message, http_response);
        return;
    }

    int user_id;
    if (getIDByCookie(cookie_it->second, user_id) == false) {
        std::string error_message = "Invalid cookie";
        errorPage(error_message, http_response);
        return;
    }
    
    replacePlusToSpace(msg_it->second);
    std::string newLineEncoded = "%0D%0A";
    std::string newLineHTML = "<br>";
    stringReplaceAll(msg_it->second, newLineEncoded, newLineHTML);
    std::string msg;
    url_escape(msg_it->second, msg);

    if (insertNewMsgToBulletin(msg, user_id)) {
        http_response.status_code = http::STATUS_SEE_OTHER;
        http_response.see_other_location = "bulletin";
        return;
    }
    else {
        std::string error_message = "Failed to interact with database, please try again later.";
        errorPage(error_message, http_response);
        return;
    }

}

bool web::WebServer::insertNewMsgToBulletin(std::string& message, int user_id) {
    sqlite3_stmt *stmt;
    const char *insert_sql = "INSERT INTO bulletin (message, user_id) VALUES (?,?);";
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] insertNewBulletin: " << sqlite3_errmsg(db);
        #endif
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, message.c_str(), message.length(), NULL);
    sqlite3_bind_int(stmt, 2, user_id);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] insertNewUser: " << sqlite3_errmsg(db) << std::endl;
        #endif
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    return true;
}

void web::WebServer::errorPage(std::string& error_message, http::HTTPResponse& http_response) {
    std::string err_page = "error.html";
    serveFile(err_page, http_response);
    std::string toReplace = "{ERROR_MESSAGE}";
    replaceString(http_response.file_data, toReplace, error_message);
}

void web::WebServer::serveFile(std::string& file_name, http::HTTPResponse& http_response) {

    std::string file_dir;

    bool forbidden = IO::isUsingParentDirectory(file_name);
    if (forbidden) {
        file_dir = ROOT_DIR + http::HTTP_ERROR_FOLDER + std::string("403.html");
        http_response.file_type = IO::TYPE_HTML;
        http_response.status_code = http::STATUS_FORBIDDEN;
        IO::readFile(file_dir, std::ios::in, http_response.file_data);
        return;
    }
    
    IO::FileType file_type;
    file_type = IO::getFileType(file_name);

    auto openmode = IO::getOpenmode(file_type);

    file_dir = ROOT_DIR + file_name;

    if (IO::readFile(file_dir, openmode, http_response.file_data)) {
        http_response.file_type = file_type;
        http_response.status_code = http::STATUS_OK;
    }
    else {
        file_dir = ROOT_DIR + http::HTTP_ERROR_FOLDER + std::string("404.html");
        http_response.file_type = IO::TYPE_HTML;
        http_response.status_code = http::STATUS_NOT_FOUND;
        IO::readFile(file_dir, std::ios::in, http_response.file_data);
    }
    return;
}

bool web::WebServer::insertNewUser(std::string& account, std::string& password) {
    sqlite3_stmt *stmt;
    const char *insert_sql = "INSERT INTO user (account, password, id_cookie) VALUES (?,?,?);";
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        
        std::cerr << "[ERROR] insertNewUser: " << sqlite3_errmsg(db) << std::endl;
        
        return false;
    }

    int tmp_cookie = 0;
    
    int j = 1;
    sqlite3_bind_text(stmt, j++, account.c_str(), account.length(), NULL);
    sqlite3_bind_text(stmt, j++, password.c_str(), password.length(), NULL);
    sqlite3_bind_int(stmt, j++, tmp_cookie);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] insertNewUser: " << sqlite3_errmsg(db) << std::endl;
        #endif
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    updateCookieByAccount(account);

    return true;
}

bool web::WebServer::getCookieByAccount(std::string account, unsigned int& cookie) {
    sqlite3_stmt *stmt;
    const char *get_cookie = "SELECT id_cookie FROM user WHERE account = ?;";
    if (sqlite3_prepare_v2(db, get_cookie, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getCookieByAccount: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }

    sqlite3_bind_text(stmt, 1, account.c_str(), account.length(), NULL);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getCookieByAccount: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }
    cookie = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return true;
}

bool web::WebServer::getAccountByID(int id, std::string& account) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT account FROM user WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getAccountByID: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    sqlite3_busy_timeout(db, db_timeout);
    sqlite3_step(stmt);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getAccountByID: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }
        
    account = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));

    sqlite3_finalize(stmt);
    return true;
}

bool web::WebServer::updateCookieByAccount(std::string account) {
    int uid;
    if (getIDByAccount(account, uid) == false)
        return false;

    time_t now;
    time(&now);
    now += 259200;

    unsigned int cookie = now;
    cookie <<= 1;
    cookie += uid;

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE user SET id_cookie = ? WHERE account = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] updateCookieByAccount: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }
    
    int j = 1;
    sqlite3_bind_int64(stmt, j++, cookie);
    sqlite3_bind_text(stmt, j++, account.c_str(), account.length(), NULL);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] updateCookieByAccount: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }
    
    sqlite3_finalize(stmt);
    return true;
}

bool web::WebServer::getIDByAccount(std::string account, int& id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id FROM user WHERE account = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getIDByAccount: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }

    sqlite3_bind_text(stmt, 1, account.c_str(), account.length(), NULL);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getIDByAccount: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }
    id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return true;
}

bool web::WebServer::getIDByCookie(std::string& cookie, int& id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id FROM user WHERE id_cookie = ? ;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getIDByCookie prepare: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }

    unsigned int cookie_t = std::stoll(cookie);
    sqlite3_bind_int64(stmt, 1, cookie_t);

    sqlite3_busy_timeout(db, db_timeout);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        #ifdef VERBOSE
        std::cerr << "[ERROR] getIDByCookie step: " << sqlite3_errmsg(db) << std::endl;
        #endif
        return false;
    }

    id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return true;
}