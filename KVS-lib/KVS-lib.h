#ifndef KVS_LIB_H
#define KVS_LIB_H
#define SERVER_ADDRESS "/tmp/server"

int sock;
int establish_connection (const char * group_id, const char * secret);
int put_value(char * key, char * value);
int get_value(char * key, char ** value);
int delete_value(char * key);
int register_callback(char * key, void (*callback_function)(char *));
int close_connection();

#endif
