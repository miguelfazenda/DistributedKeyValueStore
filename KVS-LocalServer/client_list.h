#ifndef __CLIENTS_LIST_H
#define __CLIENTS_LIST_H

#include <stdbool.h>
#include <pthread.h>

#include "../shared/message.h"

typedef struct Client_struct
{
    int sockFD;     
    pthread_t thread;   
    bool stay_connected;

    //Used when the client registers a callback
    int callback_sock_fd;

    char session_id[SESSION_ID_STR_SIZE];

    //If group_id is NULL, the client isn't logged in yet
    char* group_id;

    bool connected;
    time_t time_connected;
    time_t time_disconnected;

    pid_t pid;

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