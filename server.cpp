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
    http::getResponseMessage(http_response, message);

    write(conn_fd , message.c_str() , message.length());
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
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
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
    if (bind(server_fd, (struct sockaddr *)&address, addr_len) < 0) {
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
    if ((conn_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addr_len)) < 0) {
        std::cerr << "Failed to accept new connection.";
        exit(EXIT_FAILURE);
    }
    return true;
}

void HTTPServer::start() {
    long read_len;
    char buffer[BUFFER_SIZE] = {0};
    while (1) {
        std::cout << "------------------Wait for connection-------------------\n";
        int conn_fd;
        if (acceptConnection(conn_fd)) {
            read_len = read(conn_fd , buffer, BUFFER_SIZE);
            std::string request;
            request.assign(buffer, read_len);
            thread_pool->submit(std::bind(&response_thread, conn_fd, request));
            /*
            request.assign(buffer, read_len);
            std::string response = handler->getResponse(request);
            write(conn_fd , response.c_str() , response.length());
            close(conn_fd);
            */
            std::cout << "------------------Hello message sent-------------------\n";
            
        }
    }
}

