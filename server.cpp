#include "server.h"
#include "string.h"
#include "http.h"
#include <iostream>

void response_thread(int conn_fd, std::string request) {
    http::HTTPRequest http_request = {};
    http::HTTPResponse http_response = {};
    http::parseHttpRequest(request, http_request);

    http::getFile(http_request.file_name, http_response);

    std::string message;
    http::getResponseHeader(http_response, message);

    write(conn_fd , message.c_str() , message.length());
    write(conn_fd , http_response.file_data.c_str() , http_response.file_data.length());
    close(conn_fd);
}

HTTPServer::HTTPServer(int port) : port(port) {
    thread_pool = new tp::ThreadPool();
    // handler = new RequestHandler(root_dir);
    initAddress();
    createSocket();
    bindSocket();
    listenSocket();
}

void HTTPServer::initAddress() {
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    memset(server_address.sin_zero, '\0', sizeof(server_address.sin_zero));
    return;
}

void HTTPServer::createSocket() {
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Failed to create socket";
        exit(EXIT_FAILURE);
    }
    return;
}

void HTTPServer::bindSocket() {
    if (bind(server_fd, (struct sockaddr *)&server_address, addr_len) < 0) {
        std::cerr << "Failed to bind to socket";
        exit(EXIT_FAILURE);
    }
    return;
}

void HTTPServer::listenSocket() {
    if (listen(server_fd, max_queue) < 0) {
        std::cerr << "Failed to listen to socket";
        exit(EXIT_FAILURE);
    }
    return;
}

bool HTTPServer::acceptConnection(int& conn_fd) {
    if ((conn_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addr_len)) < 0) {
        std::cerr << "Failed to accept new connection.";
        exit(EXIT_FAILURE);
    }
    std::cout << "FD: " << conn_fd << '\n';
    return true;
}

void HTTPServer::start() {
    long read_len;
    int conn_fd;
    std::string request;
    char *buffer = new char[BUFFER_SIZE];
    while (1) {
        std::cout << "------------------Wait for connection-------------------\n";
        if (acceptConnection(conn_fd)) {
            read_len = recv(conn_fd , buffer, BUFFER_SIZE, 0);
            if (read_len == 0) {
                close(conn_fd);
                continue;
            }
            request.clear();
            request.assign(buffer, read_len);
            std::cout << request << '\n';
            thread_pool->submit(std::bind(&response_thread, conn_fd, request));
            /*
            request.assign(buffer, read_len);
            std::string response = handler->getResponse(request);
            write(conn_fd , response.c_str() , response.length());
            close(conn_fd);
            */
        }
    }
    delete []buffer;
}

