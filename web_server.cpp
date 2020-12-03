#include "web_server.h"

Web::Web() {
    int err;
    err = sqlite3_open(db_name.c_str(), &db);
    if (err) {
        std::runtime_error("Failed to open database.");
        exit(EXIT_FAILURE);
    }
    service.insert({"/login", &Web::login});
    service.insert({"/signup", &Web::signup});
    service.insert({"/bulletin", &Web::bulletin});
}

Web::~Web() {
    sqlite3_close(db);
}

void replaceString(std::string& original, std::string& oldString, std::string& newString) {
    auto pos = original.find(oldString);
    original.replace(pos, oldString.length(), newString);
}

void Web::render(std::string& request, http::HTTPResponse& http_response) {
    http::HTTPRequest http_request = {};
    http::parseHttpRequest(request, http_request);

    auto it = service.find(http_request.file_name);
    if (it != service.end()) {
        (this->*(it->second))(http_request, http_response);
    }
    else {
        Web::serveFile(http_request.file_name, http_response);
    }

    http::getResponseHeader(http_response);
    std::cerr << http_response.header << '\n';
}

void Web::login(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    if (http_request.http_method == http::METHOD_GET) {
        auto it = http_request.cookie.find("uuid");
        if (it != http_request.cookie.end()) {
            http_response.status_code = http::STATUS_SEE_OTHER;
            http_response.see_other_location = "/index.html";
            return;
        }
        std::string file_name = "/login.html";
        serveFile(file_name, http_response);
    }
    else if (http_request.http_method == http::METHOD_POST) {
        auto account_it = http_request.post_value.find("account");
        auto password_it = http_request.post_value.find("password");
        
        if (account_it == http_request.post_value.end() || 
            password_it == http_request.post_value.end()) {
            std::string error_message = "Wrong arguments in post request";
            errorPage(error_message, http_response);
            return;
        }

        const char *get_password_sql = "SELECT password FROM user WHERE account == ? ;";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, get_password_sql, -1, &stmt, NULL) != SQLITE_OK) {
            std::string file_dir = http::HTTP_ERROR_FOLDER + std::string("/500.html");
            serveFile(file_dir, http_response);
            return;
        }

        sqlite3_bind_text(stmt, 1, account_it->second.c_str(), account_it->second.length(), NULL);
        
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
                http_response.see_other_location = "/index.html";
                
                std::string cookie;
                if (getCookieByAccount(account_it->second, cookie)) {
                    http_response.cookie_expire_hour = 3;
                    http_response.cookie.insert({"uuid", cookie});
                }
            }
            else {
                std::string error_message = "Password doesn't match";
                errorPage(error_message, http_response);
                return;
            }
        }
    }
}

bool Web::getCookieByAccount(std::string& account, std::string& cookie) {
    sqlite3_stmt *stmt;
    const char *get_cookie = "SELECT id_cookie FROM user WHERE account == ? ;";
    if (sqlite3_prepare_v2(db, get_cookie, -1, &stmt, NULL) != SQLITE_OK) {
        std::runtime_error("Failed to prepare sql");
        return false;
    }

    sqlite3_bind_text(stmt, 1, account.c_str(), account.length(), NULL);
    sqlite3_step(stmt);
    cookie = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    sqlite3_finalize(stmt);
    return true;
}

void Web::signup(http::HTTPRequest& http_request, http::HTTPResponse& http_response) {
    if (http_request.http_method == http::METHOD_GET) {
        std::string file_name = "/signup.html";
        serveFile(file_name, http_response);
    }
    else if (http_request.http_method == http::METHOD_POST) {
        auto account_it = http_request.post_value.find("account");
        auto password_it = http_request.post_value.find("password");

        if (account_it == http_request.post_value.end() || 
            password_it == http_request.post_value.end()) {
            std::string error_message = "Wrong arguments in post request";
            errorPage(error_message, http_response);
            return;
        }

        const char *check_account_sql = "SELECT id FROM user WHERE account == ? ;";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, check_account_sql, -1, &stmt, NULL) != SQLITE_OK) {
            std::string file_dir = http::HTTP_ERROR_FOLDER + std::string("/500.html");
            serveFile(file_dir, http_response);
            return;
        }
        sqlite3_bind_text(stmt, 1, account_it->second.c_str(), account_it->second.length(), NULL);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string error_message = "Account already exists";
            errorPage(error_message, http_response);
            return;
        }
        sqlite3_finalize(stmt);

        if (insertNewUser(account_it->second, password_it->second)) {
            http_response.status_code = http::STATUS_SEE_OTHER;
            http_response.see_other_location = "/signup_success.html";

            std::string cookie;
            if (getCookieByAccount(account_it->second, cookie)) {
                http_response.cookie_expire_hour = 3;
                http_response.cookie.insert({"uuid", cookie});
            }
        }
        else {
            std::string file_name = http::HTTP_ERROR_FOLDER + std::string("/500.html");
            serveFile(file_name, http_response);
        }
    }
}

bool Web::insertNewUser(std::string& account, std::string& password) {
    sqlite3_stmt *stmt;
    const char *insert_sql = "INSERT INTO user (account, password, id_cookie) VALUES (?,?,?);";
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    int j = 1;
    time_t now;
    std::stringstream ss;
    time(&now);
    now += 259200;
    ss << now;
    sqlite3_bind_text(stmt, j++, account.c_str(), account.length(), NULL);
    sqlite3_bind_text(stmt, j++, password.c_str(), password.length(), NULL);
    sqlite3_bind_text(stmt, j++, ss.str().c_str(), ss.str().length(), NULL);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

void Web::errorPage(std::string& error_message, http::HTTPResponse& http_response) {
    std::string err_page = "/error.html";
    serveFile(err_page, http_response);
    std::string toReplace = "{ERROR_MESSAGE}";
    replaceString(http_response.file_data, toReplace, error_message);
}

void Web::bulletin(http::HTTPRequest& http_request, http::HTTPResponse& response) {

}

void Web::serveFile(std::string& file_name, http::HTTPResponse& http_response) {

    std::string file_dir;

    bool forbidden = IO::isUsingParentDirectory(file_name);
    if (forbidden) {
        file_dir = ROOT_DIR + http::HTTP_ERROR_FOLDER + std::string("/403.html");
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
        file_dir = ROOT_DIR + http::HTTP_ERROR_FOLDER + std::string("/404.html");
        http_response.file_type = IO::TYPE_HTML;
        http_response.status_code = http::STATUS_NOT_FOUND;
        IO::readFile(file_dir, std::ios::in, http_response.file_data);
    }
    return;
    
}