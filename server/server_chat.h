#ifndef SERVER_CHAT_H
#define SERVER_CHAT_H

#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "common/protocol.h"
#include "common/netutils.h"
#include "server/server_ipc.h"
#include "server/server_db.h"



void run_chat_session(int client_socket, int downlink_read_fd, int uplink_write_fd, const char *username, char *db_path);

#endif