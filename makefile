CC=g++
CPPFLAGS=-Wall -std=c++17 -O2
LDFLAGS=-pthread -pedantic
DEVFLAGS=-DTHREADSAFE=1 -l sqlite3
DFLAGS=-DVERBOSE

dev:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) $(DEVFLAGS) main.cpp http_server.cpp web_server.cpp http.cpp IO.cpp -o main

stable:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp http_server.cpp web_server.cpp http.cpp IO.cpp -o main

server:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp http_server.cpp http.cpp -o main

clean:
	rm -f client main