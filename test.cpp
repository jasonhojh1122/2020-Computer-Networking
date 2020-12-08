#include <iostream>
#include <string>
#include <curl/curl.h>
#include <algorithm>

bool isDigits(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

int main(int argc, char const *argv[]) {
    std::string a = "1234567890124564";
    std::cout << isDigits(a);
}