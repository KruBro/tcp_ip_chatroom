#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include <stdio.h>
#include <string.h>
#include "common/validation.h"
#include "common/protocol.h"
#include "common/netutils.h"

void display_chat_menu();

void prompt_login_credentials(SignInType *req);
ChatType prompt_chat_command(Msg *msg, const char *own_username);
void display_incoming_message(const Msg *msg);
void print_message(ReplyCode code);

#endif