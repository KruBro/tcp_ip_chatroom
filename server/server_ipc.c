#include "server_ipc.h"
#include <string.h>
#include <stdio.h>

static ClientEntry clients[MAX_CLIENTS];
static int client_count = 0;

int add_client(pid_t pid, int downlink_write_fd)
{
    // find the next free slot (question: scan for an empty slot with pid == 0,
    // or just append at client_count and increment, assuming no removal-reuse yet?)
    // fill in pid, downlink_write_fd
    // username[0] = '\0', logged_in = 0
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].pid == 0)
        {
            clients[i].pid = pid;
            clients[i].downlink_write_fd = downlink_write_fd;
            clients[i].username[0] = '\0';
            clients[i].logged_in = 0;
            return 1;
        }
    }
    return 0;
}

void remove_client(pid_t pid)
{
    // scan the array for the entry matching this pid
    // once found, decide: clear its fields to a sentinel "empty" state
    // (pid = 0, downlink_write_fd = -1, username[0] = '\0', logged_in = 0)?
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].pid == pid)
        {
            close(clients[i].downlink_write_fd);
            clients[i].pid = 0;
            clients[i].downlink_write_fd = -1;
            clients[i].username[0] = '\0';
            clients[i].logged_in = 0;
            return;
        }
    }
}

void mark_client_logged_in(pid_t pid, const char *username)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].pid == pid)
        {
            strcpy(clients[i].username, username);
            clients[i].logged_in = 1;
            return;
        }
    }
    printf("[INFO] : NOT Found\n");
}

int find_downlink_fd(const char *username, int *out_fd)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(clients[i].logged_in == 1 && strcmp(clients[i].username, username) == 0)
        {
            *out_fd = clients[i].downlink_write_fd;
            return 1;
        }
    }
    return 0;
}

int get_online_downlink_fds(pid_t exclude_pid, int *out_fds, int max_count)
{
    int found = 0;
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(found >= max_count)
            break;
        
        if(clients[i].logged_in == 1 && clients[i].pid != exclude_pid)
        {
            out_fds[found] = clients[i].downlink_write_fd;
            found++;
        }
    }
    return found;
}