CC=g++
CPPFLAGS=-Wall -std=c++17 -O2
LDFLAGS=-pthread -pedantic -l sqlite3
DFLAGS=-DTHREADSAFE=1

dev:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) -D VERBOSE main.cpp http_server.cpp http.cpp -o main

server:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp http_server.cpp http.cpp -o main

clean:
	rm -f client main