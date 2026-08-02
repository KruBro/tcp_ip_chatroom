#ifndef _XOPEN_SOURCE 
#define _XOPEN_SOURCE 700 
#endif
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
#include "server/server_auth.h"
#include "server/server_ipc.h"

#define DB_PATH "server_dev.db"

void signal_handler(int signum)
{
    int wstatus;
    pid_t pid;
    while((pid = waitpid(-1, &wstatus, WNOHANG)) > 0)
    {
        remove_client(pid);
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

    int uplink_fd[2];
    if(pipe(uplink_fd) < 0)
    {
        perror("pipe");
        return 1;
    }

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
        printf("[INFO] : Client Port ->%d\n", client_port);

        int downlink_fd[2];
        if(pipe(downlink_fd) < 0)
        {
            perror("pipe");
            close(client_socket);
            continue;
        }

        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork");
            break;
        }
        else if(pid == 0)
        {
            close(sockfd);
            close(uplink_fd[0]);
            close(downlink_fd[1]);
            printf("Child with PID -> %d\n", getpid());
            printf("Parent of the child -> %d\n", getppid());
            SignInType request;
            Reply status;
            int read_ret = readn(client_socket, REQUEST_WIRE_SIZE, (char*)&request);
            if(read_ret != REQUEST_WIRE_SIZE)
            {
                close(sockfd);
                _exit(0);
            }
            status = handle_auth_request(&request, DB_PATH, client_socket);
            writen(client_socket, REPLY_WIRE_SIZE, (char*)&status);
            
            _exit(0);
        }
        else if(pid > 0)
        {
            close(client_socket);
            close(downlink_fd[0]);

            add_client(pid, downlink_fd[1]);
        }
    }

    close(sockfd);
    return 0;
}