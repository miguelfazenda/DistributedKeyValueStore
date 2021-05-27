#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdio.h>
#include <stdint.h>

#define MSG_LOGIN 1
#define MSG_PUT 2
#define MSG_GET 3
#define MSG_DELETE 4
#define MSG_REGISTER_CALLBACK 5
#define MSG_CALLBACK 6
#define MSG_OKAY 7

#define ERROR_WRONG_SECRET -2
#define ERROR_FAILED_AUTHENTICATION -3


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

int receive_message(int sockFD, Message* msg);
int send_message(int sockFD, Message msg);
//int send_message_direct(int sockFD, __int8_t messageID, const char* firstArg, const char* secondArg);
void free_message(Message* msg);

#endif