#ifndef __AUTH_H
#define __AUTH_H

#include <sys/un.h>
#include <stdbool.h>
#include <stdint.h>

#define AUTH_CLIENT_ADDRESS "/tmp/server_auth_client"
#define AUTH_SERVER_ADDRESS "/tmp/auth_server"

int auth_sock;
struct sockaddr_un auth_sock_addr;
struct sockaddr_un auth_server_address;

int auth_create_socket();
int8_t auth_send_login(const char* group_id, const char* group_secret);
int8_t auth_create_group(const char* group_id, const char* group_secret);

#endif