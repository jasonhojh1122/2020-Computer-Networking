#include "http_server.h"
#include <sstream>
#include <cstdlib>

const char* ROOT_DIR = "./web";

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        throw std::runtime_error("Please specify the server port to listen on.");
        exit(EXIT_FAILURE);
    }
    std::stringstream ss;
    ss << argv[1];
    int port;
    ss >> port;


    HTTPServer httpServer = {port};
    httpServer.start();
    return 0;
}
