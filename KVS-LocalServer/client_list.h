#ifndef __CLIENTS_LIST_H
#define __CLIENTS_LIST_H

#include <stdbool.h>
#include <pthread.h>

typedef struct Client_struct
{
    int sockFD;
    pthread_t thread;
    bool stay_connected;
    char* group_id;

    struct Client_struct* next;
} Client;

typedef struct Client_List_struct
{
    Client* client_list;
    pthread_mutex_t mtx_client_list;
} Client_List;

void client_list_add(Client_List* list, Client* client);
void client_list_remove_and_free(Client_List* list, Client* client);

#endif