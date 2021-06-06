#ifndef __CLIENTS_LIST_H
#define __CLIENTS_LIST_H

#include <stdbool.h>
#include <pthread.h>

#include "../shared/message.h"

typedef struct Client_struct
{
    int sockFD; //The main socket
    pthread_t thread; //The thread receiving the data.
    bool stay_connected; //If the server should or not disconnect this client
    char* group_id; //If group_id is NULL, the client isn't logged in yet

    int callback_sock_fd; //The socket used when the client registers a callback

    //Each client gets a random session ID. This is used to identify when the callback socket connects, which client it's respective to.
    char session_id[SESSION_ID_STR_SIZE];

    bool connected; //If it's connected
    time_t time_connected; //Time of connection
    time_t time_disconnected; //Time of disconnection (if disconnected)

    pid_t pid; //The PID of the client process

    //The next client in the main client list 
    struct Client_struct* next;
} Client;

typedef struct Client_List_struct
{
    Client* client_list;
    pthread_mutex_t mtx_client_list;
} Client_List;

//TODO clarificar estas 3 estruturas. Talvez se possa usar esta, e tirar o struct Client_struct* next; do Client
typedef struct Client_List_Item_struct
{
    Client* client;

    struct Client_List_Item_struct* next;
} Client_List_Item;

void client_list_add(Client_List* list, Client* client);
void client_list_remove_and_free(Client_List* list, Client* client);

#endif