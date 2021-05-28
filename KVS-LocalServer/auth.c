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
    if (auth_sock == -1)
    {
        return -1;
        auth_sock_error_occured = true;
    }
    printf("socket created\n");
    auth_sock_addr.sun_family = AF_UNIX;
    strcpy(auth_sock_addr.sun_path, AUTH_CLIENT_ADDRESS);

    if(bind(auth_sock, (struct sockaddr*)&auth_sock_addr, sizeof(auth_sock_addr)) == -1)
    {
        return -1;
        auth_sock_error_occured = true;
    }
    printf("socket with an address %s\n", AUTH_CLIENT_ADDRESS);

    //Stores the server address in a struct
    auth_server_address.sun_family = AF_UNIX;
    strcpy(auth_server_address.sun_path, AUTH_SERVER_ADDRESS);

    auth_sock_error_occured = false;

    return 1;
}

int8_t auth_send_login(const char* group_id, const char* group_secret)
{
    //Send message
    int send_status = send_auth_message(create_auth_message(MSG_AUTH_CHECK_LOGIN, group_id, group_secret));
    if(send_status != 1)
        return -1;

    //Receive response from auth server
    __int8_t response;
    recvfrom(auth_sock, &response, sizeof(response), 0, (struct sockaddr*)&auth_server_address, sizeof(auth_server_address));

    return response;
}

int8_t auth_create_group(const char* group_id, const char* group_secret)
{
    //Send message
    int send_status = send_auth_message(create_auth_message(MSG_AUTH_CREATE_GROUP, group_id, group_secret));
    if(send_status != 1)
        return -1;

    //Receive response from auth server
    __int8_t response;
    recvfrom(auth_sock, &response, sizeof(response), 0, (struct sockaddr*)&auth_server_address, sizeof(auth_server_address));

    return response;
}

/**
 * @brief Sends a message on the auth_sock to the auth_server_address (declared on globals.h),
 *          with the contents of the message (uses serialize_auth_message)
 * 
 * @param msg 
 * @return int 1 means it was sent sucessfully, other values means an error ocurred
 */
int send_auth_message(AuthMessage msg)
{
    char buf[AUTH_MSG_BUFFER_SIZE];

    //Convert the message to the buffer
    serialize_auth_message(&msg, buf);

    // Send the message
    if(sendto(auth_sock, buf, AUTH_MSG_BUFFER_SIZE, 0,  (struct sockaddr*)&auth_server_address, sizeof(auth_server_address))
        != AUTH_MSG_BUFFER_SIZE)
    {
        //Didn't send correctly
        auth_sock_error_occured = true;
        return -1;
    }

    //TODO return error
    return 1;
}