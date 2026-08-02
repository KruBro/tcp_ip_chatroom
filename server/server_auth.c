#include "server_auth.h"

int is_valid_field(const char *field, int max_len)
{
    size_t len = strlen(field);
    if (len == 0 || len >= (size_t)max_len)
        return 0;

    for (int i = 0; field[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)field[i];
        if (!isalnum(c) && c != '_')
            return 0;
    }
    return 1;
}

Reply handle_auth_request(SignInType *request, char *db_path, int client_socket)
{
    Reply status;
    if(request->loginOption == AUTH_REGISTER)
    {
        if(is_valid_field(request->userName, USERNAME_LEN) == 0)
        {
            status.reply = REPLY_WRONG_FORMAT;
            return status;
        }
        if(is_valid_field(request->userPass, PASSWORD_LEN) == 0)
        {
            status.reply = REPLY_WRONG_FORMAT;
            return status;
        }

        DbResult result = db_register(request->userName, request->userPass, db_path); 
        if(result == DB_OK)
        {
            status.reply = REPLY_REGISTER_SUCCESS;
            return status;
        }
        else if(result == DB_DUPLICATE)
        {
            status.reply = REPLY_DUPLICATE_USER;
            return status;
        }
        else if(result == DB_NOT_FOUND)
        {
            status.reply = REPLY_REGISTER_FAILURE;
            return status;
        }
        else
        {
            status.reply == REPLY_CONNECTION_FAILED;
            return status;
        }
    }
    else if(request->loginOption == AUTH_LOGIN)
    {

        if(is_valid_field(request->userName, USERNAME_LEN) == 0)
        {
            status.reply = REPLY_WRONG_FORMAT;
            return status;
        }

        if(is_valid_field(request->userPass, PASSWORD_LEN) == 0)
        {
            status.reply = REPLY_WRONG_FORMAT;
            return status;
        }

        DbResult result = db_login(request->userName, request->userPass, db_path);
        if(result == DB_OK)
        {
            db_set_user_status(request->userName, db_path, "ONLINE", client_socket);
            status.reply = REPLY_LOGIN_SUCCESS;
            return status;
        }
        else if(result == DB_WRONG_PASSWORD)
        {
            status.reply = REPLY_PASSWORD_INCORRECT;
            return status;
        }
        else if(result == DB_NOT_FOUND)
        {
            status.reply = REPLY_USER_NOT_FOUND;
            return status;
        }
        else
        {
            status.reply = REPLY_CONNECTION_FAILED;
            return status;
        }
    }

    status.reply = REPLY_CONNECTION_FAILED;
    return status;
}