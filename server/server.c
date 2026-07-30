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

void signal_handler(int signum)
{
    int wstatus;
    pid_t pid;
    while((pid = waitpid(-1, &wstatus, WNOHANG)) > 0)
    {
        if (pid > 0 && WIFEXITED(wstatus))
        {
            printf("child terminated with exit code %d.\n", WEXITSTATUS(wstatus));
        }
    }
}

int main()
{
    struct sigaction sa_sigchld;

    sa_sigchld.sa_handler = signal_handler;
    sigemptyset(&sa_sigchld.sa_mask);
    sa_sigchld.sa_flags = 0;
    sigaction(SIGCHLD, &sa_sigchld, NULL);


    struct sigaction sa_sigpipe;
    sa_sigpipe.sa_handler = SIG_IGN;
    sigemptyset(&sa_sigpipe.sa_mask);
    sa_sigpipe.sa_flags = 0;
    sigaction(SIGPIPE, &sa_sigpipe, NULL);

    //1. Create TCP/IP Socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket");
        return 1;
    }
    printf("[INFO] : Created the server Socket\n");

    //2. Bind The Ip address and port number
    struct sockaddr_in serverinfo;
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(6333);
    serverinfo.sin_addr.s_addr = inet_addr("127.0.0.1");

    if((bind(sockfd, (const struct sockaddr *)&serverinfo, sizeof(serverinfo))) < 0)
    {
        perror("bind");
        return 1;
    }
    printf("[INFO] : Local Network has been created to %d socket\n", sockfd);

    //3. Fixing the queue size
    if(listen(sockfd, 10) < 0)
    {
        perror("listen");
        return 1;
    }
    printf("[INFO] : Server is listening\n");

    while(1)
    {
        struct sockaddr_in clientinfo;
        socklen_t client_len = sizeof(clientinfo);

        int client_socket = accept(sockfd, (struct sockaddr *)&clientinfo, &client_len);
        if(client_socket < 0)
        {
            perror("accept");
            continue;
        }

        char *client_ip = inet_ntoa(clientinfo.sin_addr);
        int client_port = ntohs(clientinfo.sin_port);
        printf("[INFO] : Server Accepted a Connection\n");
        printf("[INFO] : Client IP -> %s\n", client_ip);
        printf("[INFO] : Client Port ->%d", client_port);

        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork");
            break;
        }
        else if(pid == 0)
        {
            //Child_process
            //check the data that has client sent over through the send
            // login logout
            //all happens here
            char buff[TEST_MSG_LEN];
            ssize_t client_data = readn(client_socket, TEST_MSG_LEN, buff);
            if(client_data == 0)
            {
                printf("[INFO] : Client Disconnected\n");
            }
            else if(client_data < 0)
            {
                printf("[ERROR] : UNKNOWn\n");
            }
            else
            {
                printf("[SUCCESS] : Read Completed\n");
            }

            if(writen(client_socket, TEST_MSG_LEN, buff) == TEST_MSG_LEN)
            {
                printf("[EQUALS]\n");
            }
            else
                printf("[NOTEQUAL]\n");
            
            close(sockfd);
            _exit(0);
        }
        else if(pid > 0)
        {
            close(client_socket);
        }
    }

    close(sockfd);
    return 0;
}