#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>

#include "request_handler.h"

#define PORT 8080
#define MAX_QUEUE 10
#define BUFFER_SIZE 4096

class HTTPServer {
public:
    HTTPServer(const std::string& root_dir);
    void start();

private:
    const std::string root_dir;
    int server_fd;
    int max_queue;
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    RequestHandler *handler;

    void initAddress();
    void createSocket();
    void bindSocket();
    void listenSocket();
    bool acceptConnection(int& conn_fd);
    
};

#endif