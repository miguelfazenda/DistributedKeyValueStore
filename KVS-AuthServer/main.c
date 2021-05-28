#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include "../shared/message.h"
#include "../shared/hashtable.h"

#include "../shared/auth_defines.h"

#define AUTH_SERVER_ADDRESS "/tmp/auth_server"

HashTable secrets_table;
struct sockaddr_un sock_addr;
int sock;

#define RECV_BUF_SIZE 101
#define SEND_BUF_SIZE 100

void create_server(void);
void handle_message_login(AuthMessage* msg, struct sockaddr_un sender_sock_addr, socklen_t sender_sock_addr_size);
void handle_message_create_group(AuthMessage* msg, struct sockaddr_un sender_sock_addr, socklen_t sender_sock_addr_size);

int main(void)
{
    //A table that stores the GroupID-Secret
    secrets_table = table_create(free_value_str);
    table_insert(&secrets_table, "Grupo", (char*)"Secret");

    //Creates the server socket (stored in global variable "sock")
    create_server();
    
    char recv_buf[AUTH_MSG_BUFFER_SIZE];
    //ssize_t n_bytes;

    struct sockaddr_un sender_sock_addr;
    socklen_t sender_sock_addr_size;

    memset(&sender_sock_addr, 0, sizeof(struct sockaddr_un));
    while(1) {
        sender_sock_addr_size = sizeof(struct sockaddr_un);
        /*n_bytes = */recvfrom(sock, &recv_buf, AUTH_MSG_BUFFER_SIZE, 0,
                        (struct sockaddr*)&sender_sock_addr, &sender_sock_addr_size);

        AuthMessage msg;
        deserialize_auth_message(&msg, recv_buf);

        if(msg.messageID == MSG_AUTH_CHECK_LOGIN)
        {
            handle_message_login(&msg, sender_sock_addr, sender_sock_addr_size);
        }
        else if(msg.messageID == MSG_AUTH_CREATE_GROUP)
        {
            handle_message_create_group(&msg, sender_sock_addr, sender_sock_addr_size);
        }
        


        //sendto(sock, &primo, sizeof(int), 0, (struct sockaddr*)&sender_sock_addr, sender_sock_addr_size);
    }

    return 0;
}

/**
 * @brief Create the socket for the server
 */
void create_server(void)
{
    remove(AUTH_SERVER_ADDRESS);

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1){
        exit(-1);
    }
    printf("socket created\n");
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, AUTH_SERVER_ADDRESS);

    if(bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1){
        exit(-1);
    }
    printf("socket with an address %s\n", AUTH_SERVER_ADDRESS);
}

/**
 * @brief Checks if the secret is correct and sends back a response, 0 for incorrect, 1 for correct
 * 
 * @param recv_buf 
 */
void handle_message_login(AuthMessage* msg, struct sockaddr_un sender_sock_addr, socklen_t sender_sock_addr_size)
{
    printf("Check login status\n");

    char* group_id = msg->firstArg;
    char* sent_secret = msg->secondArg;
    
    /*char group_id[50];
    char sent_secret[50];

    //Copy string from buffer to local variables (\0 added to prevent a bad string crashing the server)
    memcpy(group_id, &recv_buf[1], 50);
    group_id[49] = '\0';
    memcpy(sent_secret, &recv_buf[51], 50);
    sent_secret[49] = '\0';*/



    printf("groupdID: %s\n", group_id);
    printf("secret: %s\n", sent_secret);

    int8_t loginResponse = -1;

    //Get the stored secret from the table
    char* stored_secret = (char*) table_get(&secrets_table, group_id);

    if(stored_secret == NULL)
    {
        //The table doesn't have a secret for such group
        loginResponse = AUTH_ERROR_GROUP_NOT_PRESENT;
        printf("Secret for groupID %s not present\n", group_id);
    }
    else
    {
        //There is a stored secret for this group, send 0 or 1 if the secrets match
        loginResponse = strcmp(stored_secret, sent_secret) == 0;
        printf("Secret for groupID %s %s\n", group_id, loginResponse == 1 ? "Correct" : "Incorrect");
    }
    
    //Send the response
    sendto(sock, &loginResponse, sizeof(uint8_t), 0, (struct sockaddr*)&sender_sock_addr, sender_sock_addr_size);
}

/**
 * @brief Creates a group-secret entry in the table
 * 
 * @param recv_buf 
 */
void handle_message_create_group(AuthMessage* msg, struct sockaddr_un sender_sock_addr, socklen_t sender_sock_addr_size)
{
    printf("Create group (Store secret)\n");

    char* group_id = msg->firstArg;
    char* sent_secret = msg->secondArg;

    printf("groupdID: %s\n", group_id);
    printf("secret: %s\n", sent_secret);

    //Removes the entry for that group if it already exists, and stores the new groupid-secret pair
    table_delete(&secrets_table, group_id);
    table_insert(&secrets_table, group_id, strdup(sent_secret));

    //Send response
    uint8_t response = 1;
    sendto(sock, &response, sizeof(uint8_t), 0, (struct sockaddr*)&sender_sock_addr, sender_sock_addr_size);
}