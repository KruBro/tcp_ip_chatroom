#include "client_ui.h"

static char *default_menu[] = {"REGISTER", "LOGIN"};
static char *logged_in_menu[] = {"SINGLE CHAT", "GROUP CHAT", "LIST ONLINE USERS", "LOGOUT"};

static void print_menu(char **menu, int size)
{
    for(int i = 0; i < size; i++)
    {
        printf("%d. %s\n", i + 1, menu[i]);
    }
}

void display_chat_menu() {
    print_menu(logged_in_menu, 4);
    printf("Enter your choice: ");
    fflush(stdout); // Ensure it prints before select() blocks
}

void prompt_login_credentials(SignInType *req)
{
    print_menu(default_menu, 2);
    int choice;
    printf("Enter your choice: ");
    scanf("%d", &choice);
    getchar();   // consume the leftover newline scanf leaves behind
    req->loginOption = (choice == 1) ? AUTH_REGISTER : AUTH_LOGIN;

    char username[USERNAME_LEN];
    do
    {
        printf("Enter the Username: ");
        fgets(username, USERNAME_LEN, stdin);
        username[strcspn(username, "\n")] = '\0';

        if(is_valid_field(username, USERNAME_LEN) == 0)
        {
            printf("[INVALID] : letters, digits, and underscore only\n");
        }
    } while(is_valid_field(username, USERNAME_LEN) == 0);

    char password[PASSWORD_LEN];
    do
    {
        printf("Enter the Password: ");
        fgets(password, PASSWORD_LEN, stdin);
        password[strcspn(password, "\n")] = '\0';

        if(is_valid_field(password, PASSWORD_LEN) == 0)
        {
            printf("[INVALID] : letters, digits, and underscore only\n");
        }
    } while(is_valid_field(password, PASSWORD_LEN) == 0);

    strcpy(req->userName, username);
    strcpy(req->userPass, password);
}

ChatType prompt_chat_command(Msg *msg, const char *own_username)
{
    strcpy(msg->senderName, own_username);

    // print a menu: 1) Single Chat  2) Group Chat  3) Logout
    int choice;
    scanf("%d", &choice);
    getchar();   // consume the leftover newline scanf leaves behind
    fflush(stdout);


    if(choice == 1)
    {
        char reciepent[USERNAME_LEN];
        char message[MESSAGE_LEN];

        msg->chatOption = CHAT_SINGLE;
        printf("Enter the reciever name: ");
        fgets(reciepent, USERNAME_LEN, stdin);
        reciepent[strcspn(reciepent, "\n")] = '\0';
        strcpy(msg->recieverName, reciepent);

        printf("Enter the Message : ");
        fgets(message, MESSAGE_LEN, stdin);
        message[strcspn(message, "\n")] = '\0';
        strcpy(msg->message, message);
    }

    if(choice == 2)
    {
        msg->chatOption = CHAT_GROUP;
        char message[MESSAGE_LEN];

        printf("Enter the Message : ");
        fgets(message, MESSAGE_LEN, stdin);
        message[strcspn(message, "\n")] = '\0';
        strcpy(msg->message, message);        
    }

    if(choice == 3)
        msg->chatOption = CHAT_LIST_USERS;

    if(choice == 4)
        msg->chatOption = CHAT_LOGOUT;

    return msg->chatOption;
}

void display_incoming_message(const Msg *msg)
{
    printf("\nIncoming Message from %s :\n%s\n", msg->senderName, msg->message);
}

void print_message(ReplyCode code) {
    switch(code) {
        case REPLY_LOGIN_SUCCESS:
            printf("Login successful.\n");
            break;
        case REPLY_REGISTER_SUCCESS:
            printf("Registration successful.\n");
            break;
        case REPLY_REGISTER_FAILURE:
            printf("Registration failed.\n");
            break;
        case REPLY_WRONG_FORMAT:
            printf("Error: Wrong data format.\n");
            break;
        case REPLY_DUPLICATE_USER:
            printf("Error: User already exists.\n");
            break;
        case REPLY_USER_NOT_FOUND:
            printf("Error: User not found.\n");
            break;
        case REPLY_PASSWORD_INCORRECT:
            printf("Error: Incorrect password.\n");
            break;
        case REPLY_LOGOUT_SUCCESS:
            printf("Logout successful.\n");
            break;
        case REPLY_LOGOUT_FAILURE:
            printf("Logout failed.\n");
            break;
        case REPLY_CONNECTION_FAILED:
            printf("Error: Connection failed.\n");
            break;
        default:
            printf("Unknown reply code received.\n");
            break;
    }
}