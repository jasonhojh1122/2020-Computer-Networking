#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <functional>

#include <string>
#include <iostream>

#include "thread_pool.h"

#define MAX_QUEUE 10
#define MAX_CONNECTION 1000

extern char* root_dir;

typedef std::function<void(int)> ThreadFunct;

class TCPServer {
public:
    TCPServer(int port, ThreadFunct thread_funct);
    ~TCPServer();
    void start();

private:
    int port;
    int hello_fd, epoll_fd;
    int max_queue;
    struct sockaddr_in server_address;
    socklen_t addr_len = sizeof(server_address);
    ThreadFunct thread_funct;

    struct epoll_event tmp_event;
    struct epoll_event *events;

    tp::ThreadPool *thread_pool;

    void initAddress();
    void createSocket();
    void bindSocket();
    void listenSocket();
    void createEpoll();
    void addToEpoll(int fd, int op, epoll_event* event);
    void acceptConnection();
    
};

#endif