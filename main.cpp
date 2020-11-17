#include "server.h"

#define ROOT "./web"

int main(int argc, char const *argv[]) {
    HTTPServer httpServer = {ROOT};
    httpServer.start();
}
