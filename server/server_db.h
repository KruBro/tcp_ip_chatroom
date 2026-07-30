#ifndef SERVER_DB_H
#define SERVER_DB_H

typedef enum{
    DB_OK,
    DB_DUPLICATE,
    DB_NOT_FOUND,
    DB_WRONG_PASSWORD
}DbResult;

DbResult db_register(char *userName, char *password, char *path);
DbResult db_login(char *userName, char *password, char *path);

#endif