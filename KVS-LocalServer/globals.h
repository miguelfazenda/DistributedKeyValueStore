#ifndef __GLOBALS_H
#define __GLOBALS_h

#include "../shared/hashtable.h"
#include "client_list.h"

HashTable groups_table;
Client_List connected_clients; //TODO mudar o nome desta variavel, pq agora pode guardar não só os clientes conectados, mas tb os disconectados

#endif