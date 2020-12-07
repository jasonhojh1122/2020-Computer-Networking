#include "http_server.h"
#include <sstream>
#include <cstdlib>

char* ROOT_DIR;

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " [port] [root dir]\n";
        throw std::runtime_error("Argument error.");
        exit(EXIT_FAILURE);
    }

    std::stringstream ss;
    ss << argv[1];
    int port;
    ss >> port;

    int count = 0; 
    while(argv[2][++count]);

    ROOT_DIR = new char[count+2];
    for (auto i = 0; i < count; ++i) {
        ROOT_DIR[i] = argv[2][i];
    }
    if (ROOT_DIR[count] == 0) {
        ROOT_DIR[count] = '/';
        ROOT_DIR[count+1] = 0;
    }

    std::cout << "Server Root: " << ROOT_DIR << '\n';

    HTTPServer httpServer = {port};
    httpServer.start();
    return 0;
}
