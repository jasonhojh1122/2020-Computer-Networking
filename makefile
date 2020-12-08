CC=g++
CPPFLAGS=-Wall -std=c++17
LDFLAGS=-pthread -pedantic
LFLAGS= -lsqlite3 -lcurl
DFLAGS=-DVERBOSE -DSQLITE_THREADSAFE=2

dev:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp http_server.cpp web_server.cpp http.cpp IO.cpp -o main $(LFLAGS)

stable:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp http_server.cpp web_server.cpp http.cpp IO.cpp -o main

server:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp http_server.cpp http.cpp -o main

clean:
	rm -f client main