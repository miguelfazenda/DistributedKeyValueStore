/**
 * @file auth.c
 * @author your name (you@domain.com)
 * @brief This module is responsible for communicating with the AuthServer
 * @date 2021-05-27
 */

#include "auth.h"

#include <stdio.h>
#include <sys/socket.h>

#include "../shared/auth_defines.h"
#include "../shared/message.h"

int auth_create_socket()
{
    //Removes the previous socket file
    remove(AUTH_CLIENT_ADDRESS);

    auth_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (auth_sock == -1){
        return -1;
    }
    printf("socket created\n");
    auth_sock_addr.sun_family = AF_UNIX;
    strcpy(auth_sock_addr.sun_path, AUTH_CLIENT_ADDRESS);

    if(bind(auth_sock, (struct sockaddr*)&auth_sock_addr, sizeof(auth_sock_addr)) == -1){
        return -1;
    }
    printf("socket with an address %s\n", AUTH_CLIENT_ADDRESS);

    //Stores the server address in a struct
    auth_server_address.sun_family = AF_UNIX;
    strcpy(auth_server_address.sun_path, AUTH_SERVER_ADDRESS);

    return 1;
}

int8_t auth_send_login(const char* group_id, const char* group_secret)
{
    char buf[AUTH_MSG_BUFFER_SIZE];

    //Convert the message to the buffer
    AuthMessage msg = create_auth_message(MSG_AUTH_CREATE_GROUP, group_id, group_secret);
    serialize_auth_message(&msg, buf);

    // Send the message
    sendto(auth_sock, buf, AUTH_MSG_BUFFER_SIZE, 0, (const struct sockaddr *)&auth_server_address, sizeof(auth_server_address));

    //Receive response from auth server
    __int8_t response;
    recvfrom(auth_sock, &response, sizeof(response), 0, NULL, NULL);

    return response;
}

int8_t auth_create_group(const char* group_id, const char* group_secret)
{
    char buf[AUTH_MSG_BUFFER_SIZE];

    //Convert the message to the buffer
    AuthMessage msg = create_auth_message(MSG_AUTH_CREATE_GROUP, group_id, group_secret);
    serialize_auth_message(&msg, buf);

    // Send the message
    sendto(auth_sock, buf, AUTH_MSG_BUFFER_SIZE, 0, (const struct sockaddr *)&auth_server_address, sizeof(auth_server_address));

    //Receive response from auth server
    __int8_t response;
    recvfrom(auth_sock, &response, sizeof(response), 0, NULL, NULL);

    return response;
}