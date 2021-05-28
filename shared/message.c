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
    ssize_t n_bytes = recv(sockFD, buf1, 5, 0);
    if(n_bytes <= 0)
        return -1;

    msg->messageID = (int8_t)buf1[0];

    uint16_t firstArgSize = (uint16_t)buf1[1];
    uint16_t secondArgSize = (uint16_t)buf1[3];


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
    uint16_t firstArgSize = msg.firstArg == NULL ? 0 : (uint16_t)(strlen(msg.firstArg)+1);
    uint16_t secondArgSize = msg.secondArg == NULL ? 0 : (uint16_t)(strlen(msg.secondArg)+1);

    //Copies the arg sizes to the buffer
    memcpy(buffer_msg_header + 1, &firstArgSize, sizeof(uint16_t));
    memcpy(buffer_msg_header + 3, &secondArgSize, sizeof(uint16_t));

    //Send the message header
    ssize_t n_bytes = send(sockFD, buffer_msg_header, 5, 0);
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

void serialize_auth_message(AuthMessage* message, char* buffer)
{
    buffer[0] = (char)message->messageID;
    // Copy the strings to the buffer positions
    memcpy(&buffer[1], message->firstArg, 100);
    memcpy(&buffer[101], message->secondArg, 100);

    //Make sure they are null-terminated strings (even if it cuts the data)
    buffer[100] = '\0';
    buffer[200] = '\0';

    /*//Get the size of the strings (with \0), and warn if they are too big
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
    }*/
}

void deserialize_auth_message(AuthMessage* message, char* buffer)
{
    message->messageID = (int8_t)buffer[0];
    //Copy string from buffer to local variables (\0 added to prevent a bad string crashing the server)
    memcpy(message->firstArg, &buffer[1], 100);
    message->firstArg[100] = '\0';
    memcpy(message->secondArg, &buffer[101], 100);
    message->secondArg[200] = '\0';
}

AuthMessage create_auth_message(const int8_t message_id, const char* first_arg, const char* second_arg)
{
    AuthMessage msg = { .messageID = message_id };

    //Copies the strings to the message structure.
    //strncat is used because it makes sure no more the the size of the string will be written
    msg.firstArg[0] = '\0';
    strncat(msg.firstArg, first_arg, sizeof(msg.firstArg)-1);
    msg.secondArg[0] = '\0';
    strncat(msg.secondArg, second_arg, sizeof(msg.secondArg)-1);

    return msg;
}