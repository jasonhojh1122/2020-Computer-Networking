#include "http.h"

void http::parseHttpRequest(std::string& request, HTTPRequest& http_request) {
    std::string tmp;
    std::stringstream header(request);
    while (std::getline(header, tmp, '\n')) {
        http_request.parsed_request.push_back(tmp);
    }
    parseFirstRow(http_request);
    parseCookie(http_request);
    if (http_request.http_method == METHOD_POST) 
        parsePostArg(http_request);

}

void http::parseFirstRow(HTTPRequest& http_request) {
    std::stringstream ss(http_request.parsed_request[0]);
    std::string tmp;

    std::getline(ss, tmp, ' ');

    if (tmp.compare("GET") == 0)
        http_request.http_method = METHOD_GET;
    else if (tmp.compare("POST") == 0)
        http_request.http_method = METHOD_POST;
    else 
        http_request.http_method = METHOD_UNDEFINED;

    std::getline(ss, tmp, ' ');
    std::stringstream services(tmp);
    std::getline(services, tmp, '/');
    while (std::getline(services, tmp, '/')) {
        http_request.services.push_back(tmp);
    }
    return;
}

void http::parsePostArg(HTTPRequest& http_request) {
    std::stringstream ss(http_request.parsed_request.back());
    std::string post_pair;
    while (std::getline(ss, post_pair, '&')) {
        std::string arg, value;
        std::stringstream tmp(post_pair);
        std::getline(tmp, arg, '=');
        std::getline(tmp, value, '=');
        http_request.post_value.insert( std::pair<std::string, std::string>(arg, value) );
    }
}

void http::parseCookie(HTTPRequest& http_request) {
    auto it = http_request.parsed_request.begin();
    for (; it != http_request.parsed_request.end(); it++) {
        std::stringstream ss(*it);
        std::string head;
        std::getline(ss, head, ' ');
        if (head.compare("Cookie:") == 0) {
            std::string cookie_pair;
            while (std::getline(ss, cookie_pair, ' ')) {
                auto equal = cookie_pair.find('=');
                std::string cookie = cookie_pair.substr(0, equal);
                std::string value;
                auto semi = cookie_pair.find(';');
                if (semi == std::string::npos)
                    value = cookie_pair.substr(equal+1, cookie_pair.length() - equal - 1);
                else
                    value = cookie_pair.substr(equal+1, semi - equal - 1);
                http_request.cookie.insert({cookie, value});
            }
        }
    }

}

void http::getResponseHeader(HTTPResponse& http_response) {
    std::stringstream ss;
    ss << "HTTP/1.1 ";
    if (http_response.status_code == STATUS_OK) {
        ss << "200 OK\r\n";
    }
    else if (http_response.status_code == STATUS_NOT_FOUND) {
        ss << "404 NOT FOUND\r\n";
    }
    else if (http_response.status_code == STATUS_SEE_OTHER) {
        ss << "303 SEE OTHER\r\n";
        ss << "Location: " << http_response.see_other_location << "\r\n";
    }

    std::string expire = getHTTPDate(8, 0);
    auto it = http_response.cookie.begin();
    for (; it != http_response.cookie.end(); it++) {
        ss << "Set-Cookie: " << it->first << "=" << it->second << "; Expires=" << expire << " \r\n";
    }
    
    if (http_response.file_data.length() != 0) {
        ss << "Content-Type: ";
        switch (http_response.file_type) {
            case IO::TYPE_HTML:
                ss << "text/html\r\n";
                break;
            case IO::TYPE_JPEG:
                ss << "image/jpeg\r\n";
                break;
            case IO::TYPE_JPG:
                ss << "image/jpg\r\n";
                break;
            case IO::TYPE_PNG:
                ss << "image/png\r\n";
                break;
            case IO::TYPE_UNDEFINED:
                ss << "application/x-binary\r\n";
        }

        ss << "Content-Length: " << http_response.file_data.length() << "\r\n";
    
        ss << "\r\n";
    }

    http_response.header = ss.str();
}

std::string http::intToHTTPDayName(int day) {
    std::string dayName;
    switch (day)
    {
    case 0:
        return (dayName = "Sun");
    case 1:
        return (dayName = "Mon");
    case 2:
        return (dayName = "Tue");
    case 3:
        return (dayName = "Wed");
    case 4:
        return (dayName = "Thu");
    case 5:
        return (dayName = "Fri");
    case 6:
        return (dayName = "Sat");
    default:
        return (dayName = "Sun");
    }
}

std::string http::intToHTTPMonthName(int month) {
    std::string monName;
    switch (month)
    {
    case 0:
        return (monName = "Jan");
    case 1:
        return (monName = "Feb");
    case 2:
        return (monName = "Mar");
    case 3:
        return (monName = "Apr");
    case 4:
        return (monName = "May");
    case 5:
        return (monName = "Jun");
    case 6:
        return (monName = "Jul");
    case 7:
        return (monName = "Aug");
    case 8:
        return (monName = "Sep");
    case 9:
        return (monName = "Oct");
    case 10:
        return (monName = "Nov");
    case 11:
        return (monName = "Dec");
    default:
        return (monName = "Jan");
    }
}

std::string http::getHTTPDate(int UTC, int offset_hour) {
    time_t now;
    struct tm tm_s;
    time(&now);
    now += UTC * 3600;
    now += offset_hour * 3600;
    gmtime_r(&now, &tm_s);
    std::stringstream ss;
    ss << intToHTTPDayName(tm_s.tm_wday) << ", ";
    if (tm_s.tm_mday < 10) ss << 0;
    ss << tm_s.tm_mday << ' ';
    ss << intToHTTPMonthName(tm_s.tm_mon) << ' ';
    ss << tm_s.tm_year + 1900 << ' ';
    if (tm_s.tm_hour < 10) ss << 0;
    ss << tm_s.tm_hour << ':';
    if (tm_s.tm_min < 10) ss << 0;
    ss << tm_s.tm_min << ':';
    if (tm_s.tm_sec < 10) ss << 0;
    ss << tm_s.tm_sec;
    ss << " GMT";
    return ss.str();

}