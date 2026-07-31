#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include "server_db.h"

DbResult db_register(char *userName, char *password, char *path)
{
    FILE *fp = fopen(path, "a+");
    if(fp == NULL)
    {
        return DB_NOT_FOUND;
    }

    if(flock(fileno(fp), LOCK_EX) < 0)
    {
        perror("flock");
        fclose(fp);
        return DB_NOT_FOUND;
    }

    fseek(fp, 0, SEEK_SET);

    char buffer[256];

    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strlen(buffer) == 0)
            continue;

        char *kv_save = NULL;
        char *token = strtok_r(buffer, ":", &kv_save);

        if(token != NULL && (strcmp(token, userName) == 0))
        {
            flock(fileno(fp), LOCK_UN);
            fclose(fp);
            return DB_DUPLICATE;
        }
    }
    fprintf(fp, "%s:%s:%s:%d\n", userName, password, "OFFLINE", -1);
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    return DB_OK;
}

DbResult db_login(char *userName, char *password, char *path)
{
    FILE *fp = fopen(path, "r");
    if(fp == NULL)
    {
        return DB_NOT_FOUND;
    }

    flock(fileno(fp), LOCK_SH);

    fseek(fp, 0, SEEK_SET);
    
    char buffer[256];
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strlen(buffer) == 0)
            continue;
        
        char *kv_save =  NULL;
        char *token = strtok_r(buffer, ":", &kv_save);
        if(token != NULL && (strcmp(token, userName) == 0))
        {
            char *pass = strtok_r(NULL, ":", &kv_save);
            if(pass != NULL && (strcmp(pass, password) == 0))
            {
                flock(fileno(fp), LOCK_UN);
                fclose(fp);
                return DB_OK;
            }
            else
            {
                flock(fileno(fp), LOCK_UN);
                fclose(fp);
                return DB_WRONG_PASSWORD;
            }
        }
    }

    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    return DB_NOT_FOUND;
}

void db_set_user_status(char *userName, char *path, const char *status, int sockfd)
{
    FILE *fp_org = fopen(path, "r");
    if(fp_org == NULL)
        return;

    if(flock(fileno(fp_org), LOCK_EX) < 0)
    {
        perror("flock");
        fclose(fp_org);
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    FILE *fp_temp = fopen(temp_path, "w");
    if(fp_temp == NULL)
    {
        fclose(fp_org);
        return;
    }

    char buffer[256];
    while(fgets(buffer, sizeof(buffer), fp_org) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strlen(buffer) == 0)
            continue;

        char copy_buffer[256];
        strcpy(copy_buffer, buffer);

        char *kv_save = NULL;
        char *username = strtok_r(buffer, ":", &kv_save);
        if(username != NULL)
        {
            if(strcmp(username, userName) == 0)
            {
                char *password = strtok_r(NULL, ":", &kv_save);
                fprintf(fp_temp, "%s:%s:%s:%d\n", username, password, status, sockfd);
            }
            else
            {
                fprintf(fp_temp, "%s\n", copy_buffer);
            }
        }    
    }

    fclose(fp_temp);

    rename(temp_path, path);

    fclose(fp_org);
}

DbResult db_lookup_sockfd(char *userName, char *path, int *outSockFd)
{
    FILE *fp = fopen(path, "r");
    if(fp == NULL)
        return DB_NOT_FOUND;
    
    if(flock(fileno(fp), LOCK_SH) < 0)
    {
        perror("flock");
        fclose(fp);
        return DB_NOT_FOUND;
    }

    char buffer[256];
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strlen(buffer) == 0)
            continue;

        char *kv_save = NULL;
        char *username = strtok_r(buffer, ":", &kv_save);
        if(username != NULL && strcmp(username, userName) == 0)
        {
            char *password = strtok_r(NULL, ":", &kv_save);
            if(password == NULL)
            {
                fclose(fp);
                return DB_NOT_FOUND;
            }
            char *status = strtok_r(NULL, ":", &kv_save);
            if(status == NULL)
            {
                fclose(fp);
                return DB_NOT_FOUND;
            }
            char *sockfd = strtok_r(NULL, ":", &kv_save);
            if(sockfd == NULL)
            {
                fclose(fp);
                return DB_NOT_FOUND;
            }

            if(strcmp(status, "OFFLINE") == 0)
            {
                *outSockFd = -1;
                fclose(fp);
                return DB_OFFLINE;
            }
            else if(strcmp(status, "ONLINE") == 0)
            {
                *outSockFd = atoi(sockfd);
                fclose(fp);
                return DB_ONLINE;
            }
        }
    }

    fclose(fp);
    return DB_NOT_FOUND;
}