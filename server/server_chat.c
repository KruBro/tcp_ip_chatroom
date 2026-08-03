#include "server/server_chat.h"

void run_chat_session(int client_socket, int downlink_read_fd, int uplink_write_fd, const char *username, char *db_path)
{
    while(1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        FD_SET(downlink_read_fd, &readfds);

        int maxfd = (client_socket > downlink_read_fd) ? client_socket : downlink_read_fd;

        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if(ready < 0)
        {
            if(errno == EINTR) continue;
            perror("select");
            break;
        }

        if(FD_ISSET(client_socket, &readfds))
        {
            // the real client sent something — a Msg (chat command)
            // read it, and depending on msg.chatOption:
            IpcUplinkMsg msg;
            if(readn(client_socket, MSG_WIRE_SIZE, (char*)&msg.chat) == MSG_WIRE_SIZE)
            {
                if(msg.chat.chatOption == CHAT_SINGLE || msg.chat.chatOption == CHAT_GROUP)
                {
                    msg.type = IPC_CHAT_RELAY;
                    msg.pid = getpid();
                    int write_ret = writen(uplink_write_fd, IPC_WIRE_SIZE, (char*)&msg);
                    if(write_ret != IPC_WIRE_SIZE)
                    {
                        printf("[ERROR] : Relay failed\n");
                    }
                }
                else if(msg.chat.chatOption == CHAT_LOGOUT) 
                {
                    db_set_user_status(username, db_path, "OFFLINE", -1);
                    break;   
                }
            }
            else
            {
                db_set_user_status(username, db_path, "OFFLINE", -1);
                break;
            }
        }

        if(FD_ISSET(downlink_read_fd, &readfds))
        {
            // the parent relayed a message meant for this client —
            // read a Msg struct, writen() it straight out to client_socket
            Msg msg;
            if(readn(downlink_read_fd, MSG_WIRE_SIZE, (char*)&msg) == MSG_WIRE_SIZE)
            {
                if(writen(client_socket, MSG_WIRE_SIZE, (char*)&msg) == MSG_WIRE_SIZE)
                {
                    printf("[SUCCESS]\n");
                }
            }
        }
    }
}