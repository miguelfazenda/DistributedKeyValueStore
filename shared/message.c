#include "message.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>

/**
 * @brief Receives a message. The message must be freed later with free_message!
 * 
 * @param sockFD 
 * @param msg The message. Must be freed later with free_message!
 * @return int returns -1 if error. 
 */
int receive_message(int sockFD, Message* msg)
{
    //Receives the message header, that contains the ID and size of the arguments
    unsigned char buf1[5];
    size_t n_bytes = recv(sockFD, buf1, 5, 0);
    if(n_bytes <= 0)
        return -1;

    msg->messageID = buf1[0];

    __uint16_t firstArgSize = (__uint16_t)buf1[1];
    __uint16_t secondArgSize = (__uint16_t)buf1[3];


    //Receives the arguments and Allocate the arguments according to their size
    msg->firstArg = NULL;
    if(firstArgSize > 0)
    {
        msg->firstArg = (char*) malloc(firstArgSize * sizeof(char));
        n_bytes = recv(sockFD, (void*) msg->firstArg, firstArgSize * sizeof(char), 0);
        if(n_bytes <= 0)
            return -1;
    }

    msg->secondArg = NULL;
    if(secondArgSize > 0)
    {
        msg->secondArg = (char*) malloc(secondArgSize * sizeof(char));
        n_bytes = recv(sockFD, (void*) msg->secondArg, secondArgSize * sizeof(char), 0);
        if(n_bytes <= 0)
            return -1;
    }

    return 1;
}

int send_message(int sockFD, Message msg)
{
    char buffer_msg_header[5]; 
    buffer_msg_header[0] = msg.messageID;
    __uint16_t firstArgSize = msg.firstArg == NULL ? 0 : (__uint16_t)strlen(msg.firstArg)+1;
    __uint16_t secondArgSize = msg.secondArg == NULL ? 0 : (__uint16_t)strlen(msg.secondArg)+1;

    *(__uint16_t*)(buffer_msg_header + 1) = firstArgSize;
    *(__uint16_t*)(buffer_msg_header + 3) = secondArgSize;

    //Send the message header
    size_t n_bytes = send(sockFD, buffer_msg_header, 5, 0);
    if(n_bytes <= 0)
        return -1;
    if(msg.firstArg != NULL)
    {
        n_bytes = send(sockFD, msg.firstArg, firstArgSize*sizeof(char), 0);
        if(n_bytes <= 0)
            return -1;
    }
    if(msg.secondArg != NULL)
    {
        n_bytes = send(sockFD, msg.secondArg, secondArgSize*sizeof(char), 0);
        if(n_bytes <= 0)
            return -1;
    }
    return 1;
}

void auth_send_message()
{
    
}

/**
 * @brief Frees the message. Only frees what is inside the struct, not the *msg itself!
 * 
 * @param msg 
 */
void free_message(Message* msg)
{
    free(msg->firstArg);
    free(msg->secondArg);
}