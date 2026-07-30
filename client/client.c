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

#define TEST_MSG_LEN 20


int main()
{
    //1. Create TCP/IP Socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket");
        return 1;
    }
    printf("[INFO] : Created the server Socket\n");

    struct sockaddr_in serverinfo;
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(6333);
    serverinfo.sin_addr.s_addr = inet_addr("127.0.0.1");

    if((connect(sockfd, (const struct sockaddr*)&serverinfo, sizeof(serverinfo))) < 0)
    {
        perror("connect");
        return 1;
    }

    char buff[TEST_MSG_LEN] = "HELLO!TESTING";

    if(writen(sockfd, 5, buff) > 0)
    {
        printf("[SUCCESS] : MSG SENT\n");
    }

    if(writen(sockfd, TEST_MSG_LEN - 5, buff + 5) > 0)
    {
        printf("[SUCCESS] : MSG SENT\n");
    }

    char reply[TEST_MSG_LEN];

    readn(sockfd, TEST_MSG_LEN, reply);
    reply[TEST_MSG_LEN] = '\0';

    printf("%s\n", reply);

    close(sockfd);
    return 0;
}