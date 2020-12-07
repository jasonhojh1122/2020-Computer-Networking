#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>


int main(int argc, char const *argv[]) {
    std::cout << argv[0] << std::endl << argv[1] << std::endl;

    int count = 0; 
    while(argv[1][++count]);

    std::cout << count << std::endl;

    for (int i = 0; i < count; ++i) {
        std::cout << argv[1][i] << "END\n";
    }
}