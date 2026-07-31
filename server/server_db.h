#ifndef SERVER_DB_H
#define SERVER_DB_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

typedef enum{
    DB_OK,
    DB_DUPLICATE,
    DB_NOT_FOUND,
    DB_WRONG_PASSWORD,
    DB_ONLINE,
    DB_OFFLINE
}DbResult;

DbResult db_register(char *userName, char *password, char *path);
DbResult db_login(char *userName, char *password, char *path);
void db_set_user_status(char *userName, char *path, const char *status, int sockfd);
DbResult db_lookup_sockfd(char *userName, char *path, int *outSockFd);

#endif