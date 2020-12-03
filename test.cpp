#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>

#include "web_server.h"

std::string intToHTTPDayName(int day) {
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

std::string intToHTTPMonthName(int month) {
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

std::string getHTTPDate(int UTC) {
    time_t now;
    struct tm tm_s;
    time(&now);
    now += UTC * 3600;
    gmtime_r(&now, &tm_s);
    std::stringstream ss;
    ss << intToHTTPDayName(tm_s.tm_wday) << ", ";
    if (tm_s.tm_mday < 10) ss << 0;
    ss << tm_s.tm_mday << ' ';
    ss << intToHTTPMonthName(tm_s.tm_mday) << ' ';
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

std::string intToHTTPMonthName(int month) {
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

std::string getHTTPDate(int UTC) {
    time_t now;
    struct tm tm_s;
    time(&now);
    now += UTC * 3600;
    gmtime_r(&now, &tm_s);
    std::stringstream ss;
    ss << intToHTTPDayName(tm_s.tm_wday) << ", ";
    if (tm_s.tm_mday < 10) ss << 0;
    ss << tm_s.tm_mday << ' ';
    ss << intToHTTPMonthName(tm_s.tm_mday) << ' ';
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


int main() {
    std::cout << getHTTPDate(8);
}