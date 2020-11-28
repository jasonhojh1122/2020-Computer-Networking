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

    ROOT_DIR = new char[sizeof(argv[2])];
    for (auto i = 0; i < sizeof(argv[2]); ++i) {
        ROOT_DIR[i] = argv[2][i];
    }

    HTTPServer httpServer = {port};
    httpServer.start();
    return 0;
}
