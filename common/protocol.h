#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifndef _XOPEN_SOURCE 
#define _XOPEN_SOURCE 700 
#endif

#include <assert.h>

#define USERNAME_LEN    32
#define PASSWORD_LEN    32
#define MESSAGE_LEN     256
#define REQUEST_WIRE_SIZE   (USERNAME_LEN + PASSWORD_LEN + sizeof(int))
#define MSG_WIRE_SIZE       (MESSAGE_LEN + USERNAME_LEN + USERNAME_LEN + sizeof(int))
#define REPLY_WIRE_SIZE     (sizeof(int))


typedef enum{
    AUTH_REGISTER = 1,
    AUTH_LOGIN
} AuthStatus;

typedef enum{
    CHAT_SINGLE = 1,
    CHAT_GROUP,
    CHAT_LOGOUT
}ChatType;

typedef enum{
    REPLY_LOGIN_SUCCESS = 1,
    REPLY_REGISTER_SUCCESS,
    REPLY_REGISTER_FAILURE,
    REPLY_WRONG_FORMAT,
    REPLY_DUPLICATE_USER,
    REPLY_USER_NOT_FOUND,
    REPLY_PASSWORD_INCORRECT,
    REPLY_LOGOUT_SUCCESS,
    REPLY_LOGOUT_FAILURE,
    REPLY_CONNECTION_FAILED
}ReplyCode;

typedef struct{
    char userName[USERNAME_LEN];
    char userPass[PASSWORD_LEN];
    AuthStatus loginOption;
}SignInType;

static_assert(sizeof(SignInType) == REQUEST_WIRE_SIZE, "SignInType size does not match REQUEST_WIRE_SIZE — check for struct padding");


typedef struct Message{
    char message[MESSAGE_LEN];
    char senderName[USERNAME_LEN];
    char recieverName[USERNAME_LEN];
    ChatType chatOption;
}Msg;

static_assert(sizeof(Msg) == MSG_WIRE_SIZE, "Msg size does not match MSG_WIRE_SIZE - check for struct padding");

typedef struct Reply{
    ReplyCode reply;
}Reply;

static_assert(sizeof(Reply) == REPLY_WIRE_SIZE, "Reply size does not match REPLY_WIRE_SIZE - Check for struct padding");
#endif