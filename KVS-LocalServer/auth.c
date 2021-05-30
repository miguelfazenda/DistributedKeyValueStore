/**
 * @file auth.c
 * @author your name (you@domain.com)
 * @brief This module is responsible for communicating with the AuthServer
 * @date 2021-05-27
 */

#include "auth.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "../shared/auth_defines.h"

int send_auth_message_and_wait_response(AuthMessage send_msg, AuthMessage* response_msg);
int receive_auth_response(AuthMessage* msg);

/**
 * @brief  Creates socket to communicate with the AuthServer
 * @note   Side-effects: changes auth_sock, and sets a few variables
 * @retval 1 for success, -1 for error
 */
int auth_create_socket(const char* host_name, uint16_t host_port)
{
    auth_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (auth_sock == -1)
    {
        return -1;
        auth_sock_error_occured = true;
    }

    //Address of this client (receive on any ip and let system assign port)
    auth_sock_addr.sin_family = AF_INET;
    auth_sock_addr.sin_addr.s_addr = INADDR_ANY;
    auth_sock_addr.sin_port = htons(0);

    if(bind(auth_sock, (struct sockaddr*)&auth_sock_addr, sizeof(auth_sock_addr)) == -1)
    {
        return -1;
        auth_sock_error_occured = true;
    }

    //Address of the server
    auth_server_address.sin_family = AF_INET;
    auth_server_address.sin_port = htons(host_port);
    //Get the host address to the server, given a host name (IPv4, or domain name, etc.)
    struct hostent *hp;
    hp = gethostbyname(host_name);
    if (hp == (struct hostent *) 0)
    {
        fprintf(stderr, "%s: unknown host\n", host_name);
        return -1;
    }
    memcpy((char *) &auth_server_address.sin_addr, (char *) hp->h_addr,
        hp->h_length);

    auth_sock_error_occured = false;


    printf("Socket for auth comm open\n");

    return 1;
}

/**
 * @brief  Sends a login request to the AuthServer, and waits for the response
 * @note   
 * @param  group_id: 
 * @param  group_secret: 
 * @retval 1 for login correct, 0 incorrect, negative is an error
 */
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
 * @brief  Sends to the auth server the request to create a group, and waits the reponse
 * @return the return code, 1 means success
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

/**
 * @brief  Sends to the auth server the request to get the secret for a group, and gets the reponse.
 * @param  group_id: the id of the group
 * @param  group_secret: where the group_secret will be written to (if there is no error). Must be pre-allocated (AUTH_MESSAGE_STRUCT_ARG_SIZE chars)
 * @retval the return code, 1 means success, or ERROR_AUTH_GROUP_NOT_PRESENT
 */
int8_t auth_get_secret(const char* group_id, char* group_secret)
{
    AuthMessage response_msg;

    //Sends the MSG_AUTH_CREATE_GROUP message, and receives the response from the auth server
    int status = send_auth_message_and_wait_response(
        create_auth_message(MSG_AUTH_GET_SECRET, group_id, NULL), &response_msg);

    if(status != 1)
        return status;

    int8_t response = response_msg.messageID;

    if(response == 1)
    {
        group_secret[0] = '\0';
        strncat(group_secret, response_msg.secondArg, sizeof(response_msg.secondArg)-1);
    }

    return response;
}

/**
 * @brief  Helper function to send an AuthMessage to the server, and wait for the response
 * @note   
 * @param  send_msg: the message to be sent
 * @param  response_msg: where the incoming message will be written to
 * @retval 1 for success sending and receiving, -1 for error with communication
 */
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