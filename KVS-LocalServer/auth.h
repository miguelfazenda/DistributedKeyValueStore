#ifndef __AUTH_H
#define __AUTH_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include "globals.h"
#include "../shared/message.h"

//Socket to communicate with Auth Server
int auth_sock;
struct sockaddr_in auth_sock_addr;
struct sockaddr_in auth_server_address;

bool auth_sock_error_occured;

//Each request to the auth server should have a different request_number.
//This contains the next request_number, but is should always be used as request_number_counter++, 
// to return the request number, and then increment it.
uint8_t request_number_counter;

//A mutex to lock when we are communicating with the auth server
pthread_mutex_t mtx_auth;

int8_t auth_create_socket(const char* host_name, uint16_t host_port);
void auth_close_connection(void);
int8_t auth_send_login(const char* group_id, const char* group_secret);
int8_t auth_create_group(const char* group_id, char* group_secret);
int8_t auth_get_secret(const char* group_id, char* group_secret);

#endif