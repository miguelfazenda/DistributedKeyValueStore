#include "client_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Adds the client to the clients_list.
 *        Thread-safe
 * 
 * @param client 
 */

void client_list_add(Client_List* list, Client* client)
{
    pthread_mutex_lock(&list->mtx_client_list);
    if(list->client_list == NULL)
    {
        //The list was empty
        list->client_list = client;
    }
    else
    {
        //Adds the client to the end of the list
        Client* aux = list->client_list;

        while(aux != NULL && aux->next != NULL)
            aux = aux->next;

        aux->next = client;
    }
    pthread_mutex_unlock(&list->mtx_client_list);
}

/**
 * @brief Removes the client from to the clients_list and frees the memory.
 *        Thread-safe
 * 
 * @param client 
 */
void client_list_remove_and_free(Client_List* list, Client* client)
{
    pthread_mutex_lock(&list->mtx_client_list);

    bool removed = false;
    Client** aux = &list->client_list;
    while(*aux != NULL)
    {
        if(*aux == client)
        {
            //Found the client
            *aux = client->next;
            removed = true;
            break;
        }

        aux = &(*aux)->next;
    }
    if(!removed)
    {
        printf("Couln't remove client from list!\n");
    }

    pthread_mutex_unlock(&list->mtx_client_list);

    free(client);
}