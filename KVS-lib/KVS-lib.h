#ifndef KVS_LIB_H
#define KVS_LIB_H
#define SERVER_ADDRESS "/tmp/server"
#include "../shared/hashtable.h"
#include "../shared/message.h"

HashTable CallbackTable;
int client_session_number;

int sock;
int sock_callback;
pthread_t thread_sock_callback;

//This is a random str that the server sends when login is successful.
//It is used when we want to connect to the callback socket, so that the
// server identifies it's us.
char session_id[SESSION_ID_STR_SIZE];

int establish_connection (const char * group_id, const char * secret);
int put_value(char * key, char * value);
int get_value(char * key, char ** value);
int delete_value(char * key);
int register_callback(char * key, void (*callback_function)(char *));
int close_connection();

#endif
