#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>

#define PORT 8080
#define MAX_QUEUE 10
#define BUFFER_SIZE 30000

class HTTPServer {
public:
    HTTPServer(const std::string& root_dir);
    void start();

private:
    const std::string root_dir;
    int server_fd;
    int conn_fd;
    int max_queue;
    struct sockaddr_in address;
    int addr_len = sizeof(address);

    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

    void initAddress();
    void createSocket();
    void bindSocket();
    void listenSocket();
    bool acceptConnection();

    
    
};

#endif