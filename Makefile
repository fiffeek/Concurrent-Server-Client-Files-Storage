TARGET: netstore-client netstore-server

CC	= g++-7
CFLAGS	= -std=c++17 -Wall -Werror
LFLAGS	= -lpthread -lboost_program_options -lboost_system -lboost_filesystem

all: netstore-client netstore-server

netstore-server: src/server.cpp
	$(CC) $(CFLAGS) $^ $(LFLAGS) -o $@

netstore-client: src/client.cpp
	$(CC) $(CFLAGS) $^ $(LFLAGS) -o $@

.PHONY: clean TARGET
clean:
	rm -f netstore-server netstore-client *.o *~ *.bak