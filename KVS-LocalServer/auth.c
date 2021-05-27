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
    char buf[101];
    buf[0] = MSG_AUTH_CHECK_LOGIN;

    //Get the size of the strings (with \0), and warn if they are too big
    size_t size_group_id = strlen(group_id)+1;
    if(size_group_id > 49)
    {
        printf("GroupID value too long\n");
        return -1;
    }
    
    size_t size_group_secret = strlen(group_secret)+1;
    if(size_group_secret > 49)
    {
        printf("Group secret value too long\n");
        return -1;
    }

    // Copy the strings to the buffer positions 1-50, and 51-100
    memcpy(&buf[1], group_id, size_group_id);
    memcpy(&buf[51], group_secret, size_group_secret);

    // Send the message
    sendto(auth_sock, buf, 101, 0, (const struct sockaddr *)&auth_server_address, sizeof(auth_server_address));

    //Receive response from auth server
    __int8_t response;
    recvfrom(auth_sock, &response, sizeof(response), 0, NULL, NULL);

    return response;
}

int8_t auth_create_group(const char* group_id, const char* group_secret)
{
    char buf[101];
    buf[0] = MSG_AUTH_CREATE_GROUP;

    //Get the size of the strings (with \0), and warn if they are too big
    size_t size_group_id = strlen(group_id)+1;
    if(size_group_id > 49)
    {
        printf("GroupID value too long\n");
        return -1;
    }
    
    size_t size_group_secret = strlen(group_secret)+1;
    if(size_group_secret > 49)
    {
        printf("Group secret value too long\n");
        return -1;
    }

    // Copy the strings to the buffer positions 1-50, and 51-100
    memcpy(&buf[1], group_id, size_group_id);
    memcpy(&buf[51], group_secret, size_group_secret);

    // Send the message
    sendto(auth_sock, buf, 101, 0, (const struct sockaddr *)&auth_server_address, sizeof(auth_server_address));

    //Receive response from auth server
    __int8_t response;
    recvfrom(auth_sock, &response, sizeof(response), 0, NULL, NULL);

    return response;
}