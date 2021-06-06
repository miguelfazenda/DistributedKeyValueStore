#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "../shared/message.h"
#include "../shared/hashtable.h"
#include "../shared/auth_defines.h"

#define AUTH_SERVER_PORT 25565

//Contains the secrets for each group (groupid, secret)
HashTable secrets_table;
struct sockaddr_in sock_address;
int sock;

#define RECV_BUF_SIZE 101
#define SEND_BUF_SIZE 100

void create_server(void);
void handle_message_login(AuthMessage* msg, struct sockaddr_in sender_sock_addr);
void handle_message_create_group(AuthMessage* msg, struct sockaddr_in sender_sock_addr);
void handle_message_get_secret(AuthMessage* msg, struct sockaddr_in sender_sock_addr);
void handle_message_delete_group(AuthMessage* msg, struct sockaddr_in sender_sock_addr);
void generate_random_secret(char *str);

static volatile int keep_running = 1;

void control_c_handler(int x);

/**
 * @brief  When control-c is pressed, shutdown the socket
 */
void control_c_handler(__attribute__((unused)) int x) {
    keep_running = 0;
    printf("Closing server\n");
    shutdown(sock, SHUT_RDWR);
}

int main(void)
{
    signal(SIGINT, control_c_handler);
    
    //A table that stores the GroupID-Secret
    secrets_table = table_create(free_value_str);
    //table_insert(&secrets_table, "Grupo", (char*)"Secret");
    srand((unsigned int)time(NULL));

    //Creates the server socket (stored in global variable "sock")
    create_server();
    
    char recv_buf[sizeof(AuthMessage)];

    struct sockaddr_in sender_sock_addr;
    socklen_t sender_sock_addr_size;

    int return_code = 0;

    printf("Press CTRL-C to safely close the server.\n");

    memset(&sender_sock_addr, 0, sizeof(struct sockaddr_in));
    while(keep_running) {
        sender_sock_addr_size = sizeof(struct sockaddr_in);
        ssize_t n_bytes = recvfrom(sock, &recv_buf, sizeof(AuthMessage), 0,
                        (struct sockaddr*)&sender_sock_addr, &sender_sock_addr_size);
        
        if(n_bytes < 0)
        {
            printf("An error ocurred receiving data\n");
            return_code = -1;
            break;
        }
        else if(n_bytes == 0)
        {
            //Socket was shutdown
            return_code = 0;
            break;
        }

        //Convert the received bytes to a AuthMessage struct
        AuthMessage msg;
        deserialize_auth_message(&msg, recv_buf);

        //Run a function to handle the message with that messageID
        if(msg.messageID == MSG_AUTH_CHECK_LOGIN)
            handle_message_login(&msg, sender_sock_addr);
        else if(msg.messageID == MSG_AUTH_CREATE_GROUP)
            handle_message_create_group(&msg, sender_sock_addr);
        else if(msg.messageID == MSG_AUTH_GET_SECRET)
            handle_message_get_secret(&msg, sender_sock_addr);
        else if(msg.messageID == MSG_AUTH_DELETE_GROUP)
            handle_message_delete_group(&msg, sender_sock_addr);
    }

    close(sock);

    return return_code;
}

/**
 * @brief Create the socket for the server
 */
void create_server(void)
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1){
        exit(-1);
    }
    
    sock_address.sin_family = AF_INET;
    sock_address.sin_port = htons(AUTH_SERVER_PORT);
    sock_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock, (const struct sockaddr*)&sock_address, sizeof(sock_address)) == -1){
        exit(-1);
    }
    printf("Server running in port %d\n", AUTH_SERVER_PORT);
}

/**
 * @brief Checks if the secret is correct and sends back a response, 0 for incorrect, 1 for correct
 */
void handle_message_login(AuthMessage* msg, struct sockaddr_in sender_sock_addr)
{
    printf("Check login status\n");

    char* group_id = msg->firstArg;
    char* sent_secret = msg->secondArg;

    printf("groupdID: %s\n", group_id);
    printf("secret: %s\n", sent_secret);

    int8_t login_response = -1;

    //Get the stored secret from the table
    char* stored_secret = (char*) table_get(&secrets_table, group_id);

    if(stored_secret == NULL)
    {
        //The table doesn't have a secret for such group
        login_response = ERROR_AUTH_GROUP_NOT_PRESENT;
        printf("Secret for groupID %s not present\n", group_id);
    }
    else
    {
        //There is a stored secret for this group, send 0 or 1 if the secrets match
        login_response = strcmp(stored_secret, sent_secret) == 0;
        printf("Secret for groupID %s %s\n", group_id, login_response == 1 ? "Correct" : "Incorrect");
    }
    
    //Send the response (the messageID on the struct is used for sending the login_response code)
    AuthMessage resp_msg = { .messageID = login_response, .firstArg = {'\0'}, .secondArg = {'\0'}, .request_number = msg->request_number };
    send_auth_message(resp_msg, sock, sender_sock_addr);
}

/**
 * @brief Creates a group-secret entry in the table
 */
void handle_message_create_group(AuthMessage* msg, struct sockaddr_in sender_sock_addr)
{
    printf("Create group (Store secret)\n");

    char* group_id = msg->firstArg;
    char sent_secret[SECRET_SIZE];

    generate_random_secret(sent_secret);

    printf("groupdID: %s\n", group_id);
    printf("secret: %s\n", sent_secret);

    int8_t response = 1;

    if(table_get(&secrets_table, group_id) != NULL)
    {
        response = ERROR_AUTH_GROUP_ALREADY_EXISTS;
    }
    else
    {
        //Insert the new group-secret pair
        table_insert(&secrets_table, group_id, strdup(sent_secret));
    }


    //Send response
    AuthMessage resp_msg = { .messageID = response, .firstArg = {'\0'}, .secondArg = {'\0'}, .request_number = msg->request_number };
    strcpy(resp_msg.firstArg, sent_secret);

    send_auth_message(resp_msg, sock, sender_sock_addr);
}

/**
 * @brief Replies the secret for a certain group. messageID is 1 for success, ERROR_AUTH_GROUP_NOT_PRESENT if that group isn't found.
 *        The group_id is read on the firstArg, and the secret is sent on the secondArg
 */
void handle_message_get_secret(AuthMessage* msg, struct sockaddr_in sender_sock_addr)
{
    char* group_id = msg->firstArg;
    printf("Get secret for group \"%s\"\n", group_id);

    //Get the stored secret from the table
    char* stored_secret = (char*) table_get(&secrets_table, group_id);

    AuthMessage resp_msg = { .messageID = 0, .firstArg = {'\0'}, .secondArg = {'\0'}, .request_number = msg->request_number };

    if(stored_secret == NULL)
    {
        //The table doesn't have a secret for such group
        resp_msg.messageID = ERROR_AUTH_GROUP_NOT_PRESENT;
        printf("Secret for groupID %s not present\n", group_id);
    }
    else
    {
        //There is a stored secret for this group, send 0 or 1 if the secrets match
        resp_msg.messageID = 1;

        //Copy the secret to the message struct
        resp_msg.secondArg[0] = '\0';
        strncat(resp_msg.secondArg, stored_secret, sizeof(resp_msg.secondArg)-1);
    }
    
    //Send the response
    send_auth_message(resp_msg, sock, sender_sock_addr);
}

/**
 * @brief Deletes a certain group. messageID is 1 for success.
 *        The group_id is read on the firstArg
 */
void handle_message_delete_group(AuthMessage* msg, struct sockaddr_in sender_sock_addr)
{
    char* group_id = msg->firstArg;
    printf("Get secret for group \"%s\"\n", group_id);

    //Delete the stored secret from the table
    int status = table_delete(&secrets_table, group_id);

    AuthMessage resp_msg = { .messageID = 0, .firstArg = {'\0'}, .secondArg = {'\0'}, .request_number = msg->request_number };

    if(status == -1)
    {
        //The table doesn't have a secret for such group
        resp_msg.messageID = ERROR_AUTH_GROUP_NOT_PRESENT;
        printf("Secret for groupID %s not present\n", group_id);
    }
    else
    {
        //There is a stored secret for this group, send 0 or 1 if the secrets match
        resp_msg.messageID = 1;
    }
    
    //Send the response
    send_auth_message(resp_msg, sock, sender_sock_addr);
}


/**
 * @brief  Generates a random secret for a group. 
 * @note   
 * @param  *str: a prealocated string of size SECRET_SIZE where the value is written to
 * @retval None
 */
void generate_random_secret(char *str)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789!#$&/()=}][{";

    for (size_t n = 0; n < SECRET_SIZE-1; n++) {
        int key = rand() % (int) (sizeof(charset) - 1);
        str[n] = charset[key];
    }
    str[SECRET_SIZE-1] = '\0';
}