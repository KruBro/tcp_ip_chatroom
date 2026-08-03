#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common/netutils.h"
#include "common/protocol.h"

// Constants for connection (Adjust SERVER_IP and PORT if your setup differs)
#define SERVER_IP "127.0.0.1"
#define PORT 6333

int login_and_keep_connection(const char *username, const char *password);

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("usage: %s alice|bob\n", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "alice") == 0)
    {
        // Sender role
        int sockfd = login_and_keep_connection("alice", "pwd1");
        if(sockfd < 0) { printf("login failed\n"); return 1; }

        Msg msg;
        memset(&msg, 0, sizeof(Msg));
        msg.chatOption = CHAT_SINGLE;
        strcpy(msg.senderName, "alice");         
        strcpy(msg.recieverName, "bob");
        strcpy(msg.message, "Hello Bob, This is alice");

        if(writen(sockfd, MSG_WIRE_SIZE, (char*)&msg) != MSG_WIRE_SIZE)
        {
            printf("Failed to send message to Bob\n");
            close(sockfd);
            return 1;
        }

        // Read once more, waiting for Bob's incoming relayed reply
        if(readn(sockfd, MSG_WIRE_SIZE, (char*)&msg) == MSG_WIRE_SIZE)
        {
            printf("[SUCCESS] : MSG received from %s: %s\n", msg.senderName, msg.message);
        }

        close(sockfd);
    }
    else if(strcmp(argv[1], "bob") == 0)
    {
        // Receiver role
        int sockfd = login_and_keep_connection("bob", "pwd2");
        if(sockfd < 0) { printf("login failed\n"); return 1; }
        
        Msg msg;
        memset(&msg, 0, sizeof(Msg));
        
        // Block until Alice's message gets relayed by the server
        if(readn(sockfd, MSG_WIRE_SIZE, (char*)&msg) != MSG_WIRE_SIZE)
        {
            printf("Failed to read message from Alice\n");
            close(sockfd);
            return 1;
        }
        printf("[SUCCESS] : MSG received from %s\n", msg.senderName);

        msg.chatOption = CHAT_SINGLE;
        strcpy(msg.senderName, "bob");           
        strcpy(msg.recieverName, "alice");       
        strcpy(msg.message, "Hi Alice");
        
        writen(sockfd, MSG_WIRE_SIZE, (char*)&msg);
        close(sockfd);
    }

    return 0;
}

int login_and_keep_connection(const char *username, const char *password)
{
    // Step 1: Create the socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    // Step 2: Establish connection infrastructure 
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return -1;
    }

    // ADDED: Your connection debug logic
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        close(sockfd);
        return -1;
    }
    printf("DEBUG: connected OK\n");

    // Step 3: Package authentication details
    SignInType req;
    memset(&req, 0, sizeof(SignInType));
    req.loginOption = AUTH_LOGIN;
    strcpy(req.userName, username);
    strcpy(req.userPass, password);

    // ADDED: Your request transmission debug logic
    if(writen(sockfd, REQUEST_WIRE_SIZE, (char*)&req) != REQUEST_WIRE_SIZE)
    {
        printf("DEBUG: writen(request) failed\n");
        close(sockfd);
        return -1;
    }
    printf("DEBUG: request sent OK\n");

    // ADDED: Your server reply read debug logic
    Reply reply;
    memset(&reply, 0, sizeof(Reply));
    if(readn(sockfd, REPLY_WIRE_SIZE, (char*)&reply) != REPLY_WIRE_SIZE)
    {
        printf("DEBUG: readn(reply) failed\n");
        close(sockfd);
        return -1;
    }
    printf("DEBUG: reply.reply = %d\n", reply.reply);

    // ADDED: Your validation debug logic
    if(reply.reply != REPLY_LOGIN_SUCCESS)
    {
        printf("DEBUG: reply was not REPLY_LOGIN_SUCCESS\n");
        close(sockfd);
        return -1;
    }

    return sockfd;   // Success — Caller now owns this open socket session
}
