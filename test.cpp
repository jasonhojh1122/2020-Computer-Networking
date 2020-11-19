#include "thread_pool.h"
#include <iostream>
#include <unistd.h>
#include <sstream>


void func(std::string str) {
    std::cout << str << std::endl;
    return;
}

int main() {
    tp::ThreadPool thread_pool = {};
    int i = 0;
    for (i = 20; i > 0; i--) {
        std::stringstream ss;
        ss << "In thread: " << i;
        thread_pool.submit(std::bind(&func, ss.str()));
    }
    while(1) {
        //
    }
    return 0;
}