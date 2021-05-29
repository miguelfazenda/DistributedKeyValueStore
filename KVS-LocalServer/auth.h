#ifndef __AUTH_H
#define __AUTH_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include "globals.h"
#include "../shared/message.h"

#define AUTH_SERVER_IP "127.0.0.1"
#define AUTH_SERVER_PORT 25565

//Socket to communicate with Auth Server
int auth_sock;
struct sockaddr_in auth_sock_addr;
struct sockaddr_in auth_server_address;

bool auth_sock_error_occured;

int auth_create_socket();
int8_t auth_send_login(const char* group_id, const char* group_secret);
int8_t auth_create_group(const char* group_id, const char* group_secret);

#endif