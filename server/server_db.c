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
        char *token = __strtok_r(buffer, ":", &kv_save);

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
        char *token = __strtok_r(buffer, ":", &kv_save);
        if(token != NULL && (strcmp(token, userName) == 0))
        {
            char *pass = __strtok_r(NULL, ":", &kv_save);
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