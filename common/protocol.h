#ifndef PROTOCOL_H
#define PROTOCOL_H

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
    REPLY_DUPLICATE_USER,
    REPLY_USER_NOT_FOUND,
    REPLY_PASSWORD_INCORRECT,
    REPLY_LOGOUT_SUCCESS,
    REPLY_LOGOUT_FAILURE
}ReplyCode;

typedef struct{
    char userName[USERNAME_LEN];
    char userPass[PASSWORD_LEN];
    AuthStatus loginOption;
}SignInType;


typedef struct Message{
    char message[MESSAGE_LEN];
    char senderName[USERNAME_LEN];
    char recieverName[USERNAME_LEN];
    ChatType chatOption;
}Msg;

typedef struct Reply{
    ReplyCode reply;
}Reply;

#endif