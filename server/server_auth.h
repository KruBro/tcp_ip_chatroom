#ifndef SERVER_AUTH_H
#define SERVER_AUTH_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "server/server_db.h"
#include "common/protocol.h"
#include "common/netutils.h"

Reply handle_auth_request(SignInType *request, char *db_path, int client_socket);

#endif