CC=g++
CPPFLAGS=-Wall -std=c++17
LDFLAGS=-pthread -pedantic
LFLAGS= -lsqlite3 -lcurl
DFLAGS=-DVERBOSE -DSQLITE_THREADSAFE=2

dev:
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(DFLAGS) web_main.cpp tcp_server.cpp web_server.cpp http.cpp IO.cpp -o web_main $(LFLAGS)

clean:
	rm -f client main