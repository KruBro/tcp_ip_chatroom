#include "server_ipc.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static ClientEntry clients[MAX_CLIENTS];
static int client_count = 0;

int add_client(pid_t pid, int downlink_write_fd)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(client_count >= MAX_CLIENTS)
            break;
        
        if(clients[i].pid == 0)
        {
            clients[i].pid = pid;
            clients[i].downlink_write_fd = downlink_write_fd;
            clients[i].username[0] = '\0';
            clients[i].logged_in = 0;
            client_count++;
            return 1;
        }
    }
    return 0;
}

void remove_client(pid_t pid)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {   
        if(clients[i].pid == pid)
        {
            client_count--;
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