#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include "errno.h"
#include "common/netutils.h"
#include "common/protocol.h"
#include "common/constants.h"
#include "client/client_ui.h"

Reply send_auth_request(SignInType *req, int *out_sockfd)
{
    Reply reply;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    struct sockaddr_in serverinfo;
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(PORT);
    serverinfo.sin_addr.s_addr = inet_addr(SERVER_IP);

    if(connect(sockfd, (const struct sockaddr*)&serverinfo, sizeof(serverinfo)) < 0)
    {
        close(sockfd);
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    if(writen(sockfd, REQUEST_WIRE_SIZE, (char*)req) != REQUEST_WIRE_SIZE)
    {
        close(sockfd);
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    if(readn(sockfd, REPLY_WIRE_SIZE, (char*)&reply) != REPLY_WIRE_SIZE)
    {
        close(sockfd);
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    // question: on FAILURE (reply not success), should this function close
    // the socket itself here, or leave that to the caller? think about
    // register/wrong-password cases specifically — is there any reason
    // to keep the socket open after those?

    *out_sockfd = sockfd;
    return reply;
}

int main()
{
    SignInType req;
    prompt_login_credentials(&req);

    int sockfd;
    Reply reply = send_auth_request(&req, &sockfd);
    print_message(reply.reply);

    if(reply.reply != REPLY_LOGIN_SUCCESS)
    {
        // register success/failure, wrong password, etc. — nothing more to do
        return 0;
    }

    Msg msg;

    display_chat_menu();
    while(1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);

        int maxfd = (STDIN_FILENO > sockfd) ? STDIN_FILENO : sockfd;

        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if(ready < 0)
        {
            if(errno == EINTR)
                continue;
            
            perror("select");
            break;
        }

        if(FD_ISSET(STDIN_FILENO, &readfds))
        {
            ChatType chatType = prompt_chat_command(&msg,  req.userName);
            if(writen(sockfd, MSG_WIRE_SIZE, (char*)&msg) != MSG_WIRE_SIZE)
            {
                close(sockfd);
                printf("[ERROR] : Relay Disconnected\n");
                return 1;
            }
            if(chatType == CHAT_LOGOUT)
                break;

            display_chat_menu();
        }
        
        if(FD_ISSET(sockfd, &readfds))
        {
            if(readn(sockfd, MSG_WIRE_SIZE, (char*)&msg) != MSG_WIRE_SIZE)
            {
                printf("[ERROR] : Relay Disconnected\n");
                close(sockfd);
                return 1;
            }
            display_incoming_message(&msg);
            display_chat_menu();
        }
    }

    close(sockfd);
    return 0;
}