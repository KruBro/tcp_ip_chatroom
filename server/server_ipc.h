#ifndef SERVER_IPC_H
#define SERVER_IPC_H

#include "common/protocol.h"

#define MAX_CLIENTS 64

typedef struct {
    pid_t pid;
    int downlink_write_fd;
    char username[USERNAME_LEN];
    int logged_in;
} ClientEntry;

int add_client(pid_t pid, int downlink_write_fd);
void mark_client_logged_in(pid_t pid, const char *username);
int find_downlink_fd(const char *username, int *out_fd);
void remove_client(pid_t pid);

#endif