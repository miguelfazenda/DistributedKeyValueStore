#ifndef __GLOBALS_H
#define __GLOBALS_h

#include "../shared/hashtable.h"
#include "client_list.h"

HashTable groups_table;
Client_List connected_clients; //TODO mudar o nome desta variavel, pq agora pode guardar não só os clientes conectados, mas tb os disconectados

//This is a table that stores a list of clients that must be warned when the key is changed.
// It isn't separated by groups. We must check if each client belongs to the group
// (key, Client List(Client_Item*))
pthread_mutex_t clients_with_callback_by_key_mtx;
HashTable clients_with_callback_by_key;

#endif