#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>

#define MSG_LOGIN 1
#define MSG_PUT 2
#define MSG_GET 3
#define MSG_DELETE 4
#define MSG_REGISTER_CALLBACK 5
#define MSG_CALLBACK 6
#define MSG_OKAY 7
#define MSG_DISCONNECT 8
#define MSG_SEND_PID 9

#define SESSION_ID_STR_SIZE 20 //Including \0

#include "error_codes.h"

/*typedef struct MessageHeader_struct
{
    __uint8_t messageID;
    __uint16_t firstArgSize;
    __uint16_t secondArgSize;
} MessageHeader;*/

typedef struct Message_struct
{
    int8_t messageID;
    char* firstArg;
    char* secondArg;
} Message;

#define AUTH_MESSAGE_STRUCT_ARG_SIZE 100
typedef struct AuthMessage_struct
{
    int8_t messageID;
    char firstArg[AUTH_MESSAGE_STRUCT_ARG_SIZE];
    char secondArg[AUTH_MESSAGE_STRUCT_ARG_SIZE];

    //Contains an unique number for that request, to check if the response corresponds to this request
    uint8_t request_number;
} AuthMessage;

void serialize_auth_message(AuthMessage* message, char* buffer);
void deserialize_auth_message(AuthMessage* message, char* buffer);
AuthMessage create_auth_message(const int8_t message_id, const char* first_arg, const char* second_arg, uint8_t request_number);
int8_t send_auth_message(AuthMessage msg, int sock, struct sockaddr_in server_address);

int receive_message(int sockFD, Message* msg);
int send_message(int sockFD, Message msg);
//int send_message_direct(int sockFD, __int8_t messageID, const char* firstArg, const char* secondArg);
void free_message(Message* msg);

#endif