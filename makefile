CC=g++
CFLAGS=-Wall -std=c++17 -O2

server:
	$(CC) $(CFLAGS) server.cpp main.cpp request_handler.cpp -o main

all:
	$(CC) $(CFLAGS) server.cpp main.cpp request_handler.cpp -o main
	$(CC) $(CFLAGS) client.cpp -o client

clean:
	rm -f client main