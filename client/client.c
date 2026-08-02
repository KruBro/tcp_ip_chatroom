#define _XOPEN_SOURCE 700 
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "common/netutils.h"
#include "common/protocol.h"


void print_message(ReplyCode code) {
    switch(code) {
        case REPLY_LOGIN_SUCCESS:
            printf("Login successful.\n");
            break;
        case REPLY_REGISTER_SUCCESS:
            printf("Registration successful.\n");
            break;
        case REPLY_REGISTER_FAILURE:
            printf("Registration failed.\n");
            break;
        case REPLY_WRONG_FORMAT:
            printf("Error: Wrong data format.\n");
            break;
        case REPLY_DUPLICATE_USER:
            printf("Error: User already exists.\n");
            break;
        case REPLY_USER_NOT_FOUND:
            printf("Error: User not found.\n");
            break;
        case REPLY_PASSWORD_INCORRECT:
            printf("Error: Incorrect password.\n");
            break;
        case REPLY_LOGOUT_SUCCESS:
            printf("Logout successful.\n");
            break;
        case REPLY_LOGOUT_FAILURE:
            printf("Logout failed.\n");
            break;
        case REPLY_CONNECTION_FAILED:
            printf("Error: Connection failed.\n");
            break;
        default:
            printf("Unknown reply code received.\n");
            break;
    }
}


void send_signin_data(SignInType *req, AuthStatus option, const char *name, const char *pass)
{
    req->loginOption = option;
    strcpy(req->userName, name);
    strcpy(req->userPass, pass);
}


Reply send_auth_request(SignInType *req)
{
    Reply reply;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket");
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }
    printf("[INFO] : Created the server Socket\n");

    struct sockaddr_in serverinfo;
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(6333);
    serverinfo.sin_addr.s_addr = inet_addr("127.0.0.1");

    if((connect(sockfd, (const struct sockaddr*)&serverinfo, sizeof(serverinfo))) < 0)
    {
        perror("connect");
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    if((writen(sockfd, REQUEST_WIRE_SIZE, (char*)req)) != REQUEST_WIRE_SIZE)
    {
        close(sockfd);
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    if((readn(sockfd, REPLY_WIRE_SIZE, (char*)&reply) != REPLY_WIRE_SIZE))
    {
        close(sockfd);
        reply.reply = REPLY_CONNECTION_FAILED;
        return reply;
    }

    close(sockfd);
    return reply;

}

int main()
{
    SignInType req;
    Reply reply;

    send_signin_data(&req, AUTH_REGISTER, "Shahad", "1234");
    reply = send_auth_request(&req);
    print_message(reply.reply);

    send_signin_data(&req, AUTH_REGISTER, "Shahad", "1234");
    reply = send_auth_request(&req);
    print_message(reply.reply);

    send_signin_data(&req, AUTH_LOGIN, "Shahad", "1234");
    reply = send_auth_request(&req);
    print_message(reply.reply);

    return 0;
}