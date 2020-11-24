#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>

// #include "request_handler.h"
#include "thread_pool.h"

#define MAX_QUEUE 10
#define BUFFER_SIZE 1024

extern const char* root_dir;

class HTTPServer {
public:
    HTTPServer(int port);
    void start();

private:
    int port;
    int server_fd;
    int max_queue;
    struct sockaddr_in server_address, client_address;
    int addr_len = sizeof(client_address);

    // RequestHandler *handler;
    tp::ThreadPool* thread_pool;

    void initAddress();
    void createSocket();
    void bindSocket();
    void listenSocket();
    bool acceptConnection(int& conn_fd);
    
};

#endif