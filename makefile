CC = gcc
CFLAGS = -Wall -I.

SERVER_SRCS = server/server.c server/server_auth.c server/server_db.c server/server_ipc.c server/server_chat.c common/netutils.c common/validation.c
CLIENT_SRCS = client/client.c client/client_ui.c common/netutils.c common/validation.c
TEST_SRCS   = tests/test_db.c server/server_db.c

all: server_app client_app

server_app: $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS)

client_app: $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS)

test: $(TEST_SRCS)
	$(CC) $(CFLAGS) -o tests/test_db_app $(TEST_SRCS)
	./tests/test_db_app

clean:
	rm -f server_app client_app tests/test_db_app

.PHONY: all test clean