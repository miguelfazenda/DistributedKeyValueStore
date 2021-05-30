#include "../shared/hashtable.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "../shared/message.h"
#include "client_list.h"
#include "message_handling.h"
#include "globals.h"

#define SERVER_ADDRESS "/tmp/server"
#define AUTH_CLIENT_ADDRESS "/tmp/server_auth_client"
#define AUTH_SERVER_ADDRESS "/tmp/auth_server"

int auth_sock;
struct sockaddr_un auth_sock_addr;
struct sockaddr_un auth_server_address;

void create_server();
void create_auth_client_socket();
void remove_and_free_client_from_list(Client* client);

int main()
{
    groups_table = table_create(free_value_hashtable);
    /*table_insert(&groups_table, "miguel", "fixe");
    table_insert(&groups_table, "ab", "ola1");
    table_insert(&groups_table, "ba", "ola2");

    printf("miguel -> %s\n", (char *)table_get(&groups_table, "miguel"));
    printf("ab -> %s\n", (char *)table_get(&groups_table, "ab"));
    printf("ba -> %s\n", (char *)table_get(&groups_table, "ba"));

    table_delete(&groups_table, (void *)"ab");

    //DEvia dar um apontador para NULL, pq removemos o ab
    printf("ab -> pointer: %p\n", table_get(&groups_table, "ab"));
    printf("ab -> pointer: %p\n", table_get(&groups_table, "bbbb"));*/

/*    create_auth_client_socket();

    char buf[100];
    buf[0] = 1;
    sendto(auth_sock, buf, 100, 0, &auth_server_address, sizeof(auth_server_address));*/

    create_server();

    return 0;
}

void* thread_client_routine(void* in)
{
    Client* client = (Client*) in;
    int socketFD = client->sockFD;

    while(client->stay_connected)
    {
        Message msg;
        msg.firstArg = NULL;
        msg.secondArg = NULL;

        if(receive_message(socketFD, &msg) == 1)
        {
            printf("Received msg %d, %s, %s\n", msg.messageID, msg.firstArg, msg.secondArg);

            if(msg.messageID == MSG_PUT)
            {
                if(msg_received_put(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }
            else if(msg.messageID == MSG_LOGIN)
            {
                if(msg_received_login(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }
            else if(msg.messageID == MSG_GET)
            {
                if(msg_received_get(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }

            /*if(msg.messageID < 0 || msg.messageID > MAX_HANDLING_FUNCTION_ID || message_handling_functions[msg.messageID] == NULL)
            {
                printf("Cant find message handling function for msg id: %d", msg.messageID);
            }
            else
            {
                //Call the function to handle the message
                int ret = message_handling_functions[msg.messageID](client, &msg);
                if(ret == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }*/

        }
        else
        {
            // Error, disconnect client
            printf("Error receiving message from client. Disconnecting client A\n");
            client->stay_connected = 0;
        }

        //Free the message
        free_message(&msg);
    }

    //Client disconnected, close socket and remove it from clients list
    close(socketFD);
    client_list_remove_and_free(&connected_clients, client);

    return(NULL);
}

void client_connected(int clientFD)
{
    //Add client to the list of connected clients (connected_clients_list)
    Client* client = (Client*)malloc(sizeof(Client));
    client->next = NULL;
    client->sockFD = clientFD;
    client->group_id = NULL;
    client->stay_connected = true;

    client_list_add(&connected_clients, client);

    //Starts the client thread, and saves the thread handler in client->thread
    pthread_create(&client->thread, NULL, thread_client_routine, (void*)client);
}

void create_server()
{
    int listen_sock;
    struct sockaddr_un listen_sock_addr;

    //Removes the previous socket file
    remove(SERVER_ADDRESS);

    //Creates the socket
    listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        exit(-1);
    }
    printf("socket created\n");

    //Sets the socket address
    listen_sock_addr.sun_family = AF_UNIX;
    strcpy(listen_sock_addr.sun_path, SERVER_ADDRESS);

    //Binds the socket to the listen address
    if (bind(listen_sock, (struct sockaddr *)&listen_sock_addr, sizeof(listen_sock_addr)) == -1)
    {
        exit(-1);
    }
    printf("Socket binded with an address %s\n", SERVER_ADDRESS);

    //Listen on that socket for incoming connections
    if (listen(listen_sock, 5) == -1)
    {
        printf("Error listening\n");
        printf("Error: %s\n", strerror(errno));
        exit(-1);
    }

    while (1)
    {
        //Waits and accepts client connections
        int clientFD = accept(listen_sock, NULL, NULL);


        if (clientFD < 0)
        {
            printf("Erro accepting client\n");
            printf("Error: %s\n", strerror(errno));
            exit(-1);
        }
        else
        {
            printf("A new client has connected\n");

            client_connected(clientFD);
        }
    }
}

void create_auth_client_socket()
{
    //Removes the previous socket file
    remove(AUTH_CLIENT_ADDRESS);

    auth_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (auth_sock == -1){
        exit(-1);
    }
    printf("socket created\n");
    auth_sock_addr.sun_family = AF_UNIX;
    strcpy(auth_sock_addr.sun_path, AUTH_CLIENT_ADDRESS);

    if(bind(auth_sock, (struct sockaddr*)&auth_sock_addr, sizeof(auth_sock_addr)) == -1){
        exit(-1);
    }
    printf("socket with an address %s\n", AUTH_CLIENT_ADDRESS);

    //Stores the server address in a struct
    auth_server_address.sun_family = AF_UNIX;
    strcpy(auth_server_address.sun_path, AUTH_SERVER_ADDRESS);
}