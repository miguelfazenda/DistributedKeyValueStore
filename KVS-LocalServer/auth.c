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

int send_auth_message_and_wait_response(AuthMessage send_msg, AuthMessage* response_msg);
int receive_auth_response(AuthMessage* msg);

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
    AuthMessage response_msg;

    //Sends the MSG_AUTH_CHECK_LOGIN message, and receives the response from the auth server
    int status = send_auth_message_and_wait_response(
        create_auth_message(MSG_AUTH_CHECK_LOGIN, group_id, group_secret), &response_msg);

    if(status != 1)
        return status;

    int8_t response = response_msg.messageID;
    return response;
}

/**
 * @brief Sends to the auth server the request to create a group, and waits the reponse
 * 
 * @param group_id 
 * @param group_secret 
 * @return int8_t the return code, 1 means success
 */
int8_t auth_create_group(const char* group_id, const char* group_secret)
{
    AuthMessage response_msg;

    //Sends the MSG_AUTH_CREATE_GROUP message, and receives the response from the auth server
    int status = send_auth_message_and_wait_response(
        create_auth_message(MSG_AUTH_CREATE_GROUP, group_id, group_secret), &response_msg);

    if(status != 1)
        return status;

    int8_t response = response_msg.messageID;
    return response;
}

int send_auth_message_and_wait_response(AuthMessage send_msg, AuthMessage* response_msg)
{
    //Stores whether the send and recv worked correctly
    int status;

    //Send message
    status = send_auth_message(send_msg,
        auth_sock, auth_server_address);
    if(status != 1)
    {
        auth_sock_error_occured = true;
        return -1;
    }

    //Receive response from auth server
    status = receive_auth_response(response_msg);
    if(status != 1)
    {
        auth_sock_error_occured = true;
        return -1;
    }
    
    return 1;
}

/**
 * @brief Receives a message from sock and puts it in msg
 * 
 * @param msg where the message will be stored
 * @param sock the socket
 */
int receive_auth_response(AuthMessage* msg)
{
    char recv_buf[AUTH_MSG_BUFFER_SIZE];
    ssize_t n_bytes = recvfrom(auth_sock, &recv_buf, AUTH_MSG_BUFFER_SIZE, 0,
                        NULL, NULL);
        
    if(n_bytes < 1)
    {
        printf("An error ocurred receiving data\n");
        return -1;
    }
    deserialize_auth_message(msg, recv_buf);

    return 1;
}