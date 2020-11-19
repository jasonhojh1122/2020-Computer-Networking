CC=g++
CPPFLAGS=-Wall -std=c++17 -O2
LDFLAGS=-pthread

server:
	$(CC) $(CPPFLAGS) $(LDFLAGS) main.cpp server.cpp http.cpp -o main

all:
	$(CC) $(CPPFLAGS) server.cpp main.cpp request_handler.cpp -o main
	$(CC) $(CPPFLAGS) client.cpp -o client

clean:
	rm -f client main