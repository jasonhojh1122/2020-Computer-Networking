#include "http_server.h"

void unblockSocket(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get socket file descriptor's flags.");
        exit(EXIT_FAILURE);
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        throw std::runtime_error("Failed to set socket file descriptor's flags.");
        exit(EXIT_FAILURE);
    }
    return;
}

void response_thread(int conn_fd) {
    
    int read_len;
    char* buffer = new char[BUFFER_SIZE];
    read_len = recv(conn_fd , buffer, BUFFER_SIZE, 0);
    if (read_len == 0) {
        close(conn_fd);
        return;
    }

    std::string request;
    request.clear();
    request.assign(buffer, read_len);

    #ifdef VERBOSE
    std::cout << "----------New Request----------\n";
    std::cout << request << '\n';
    #endif

    http::HTTPResponse http_response = {};
    
    Web web = {};
    web.render(request, http_response);

    IO::sendAll(conn_fd, http_response.header);
    IO::sendAll(conn_fd, http_response.file_data);
    close(conn_fd);
    delete []buffer;
}

HTTPServer::HTTPServer(int port) : port(port) {

    initAddress();
    createSocket();
    bindSocket();
    listenSocket();
    createEpoll();
    
    thread_pool = new tp::ThreadPool();
}

HTTPServer::~HTTPServer() {
    delete thread_pool;
    delete []events;
}

void HTTPServer::initAddress() {
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    memset(server_address.sin_zero, '\0', sizeof(server_address.sin_zero));
    return;
}

void HTTPServer::createSocket() {
    if ((hello_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::runtime_error("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    return;
}

void HTTPServer::bindSocket() {
    if (bind(hello_fd, (struct sockaddr *)&server_address, addr_len) < 0) {
        std::runtime_error("Failed to bind to socket");
        exit(EXIT_FAILURE);
    }
    return;
}

void HTTPServer::listenSocket() {
    if (listen(hello_fd, max_queue) < 0) {
        std::runtime_error("Failed to listen to socket");
        exit(EXIT_FAILURE);
    }
    return;
}

void HTTPServer::createEpoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::runtime_error("Failed to create epoll.");
        exit(EXIT_FAILURE);
    }
}

void HTTPServer::addToEpoll(int fd, int op, epoll_event* event) {
    if (epoll_ctl(epoll_fd, op, fd, event) == -1) {
        std::runtime_error("Failed to add to epoll");
        exit(EXIT_FAILURE);
    }
}

void HTTPServer::acceptConnection() {

    int conn_fd;
    struct sockaddr_in client_address;
    socklen_t in_len = sizeof(client_address);

    while (1) {
        conn_fd = accept(hello_fd, (struct sockaddr *)&client_address, (socklen_t*)&in_len);
        if (conn_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            else {
                std::runtime_error("Failed to accept new connection");
                exit(EXIT_FAILURE);
            }
        }
        else {
            tmp_event.data.fd = conn_fd;
            tmp_event.events = EPOLLIN | EPOLLET;
            addToEpoll(conn_fd, EPOLL_CTL_ADD, &tmp_event);
        }
    }
}

void HTTPServer::start() {

    tmp_event.data.fd = hello_fd;
    tmp_event.events = EPOLLIN | EPOLLET;
    addToEpoll(hello_fd, EPOLL_CTL_ADD, &tmp_event);
    unblockSocket(hello_fd);

    events = new epoll_event[MAX_CONNECTION];

    std::cout << "Listening on port: " << port << "\n";
    while (1) {
        int n;
        n = epoll_wait(epoll_fd, events, MAX_CONNECTION, -1);
        for (int i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) || 
                (events[i].events & EPOLLHUP) || 
                ((events[i].events & EPOLLIN) == 0)) {
                close(events[i].data.fd);
                continue;
            }
            else if (events[i].data.fd == hello_fd) {
                acceptConnection();
            }
            else {
                int conn_fd = events[i].data.fd;
                thread_pool->submit(std::bind(&response_thread, conn_fd));
            }
        }
        
    }
}

