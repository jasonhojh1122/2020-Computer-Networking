CC=g++
CPPFLAGS=-Wall -std=c++17 -O2
LDFLAGS=-pthread -pedantic
DFLAGS=-D VERBOSE

verbose:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) main.cpp server.cpp http.cpp -o main

server:
	$(CC) $(CPPFLAGS) $(LDFLAGS) main.cpp server.cpp http.cpp -o main

clean:
	rm -f client main