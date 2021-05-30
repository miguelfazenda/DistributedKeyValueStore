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
        return (int)n_bytes;

    msg->messageID = (int8_t)buf1[0];

    //Size of the strings, including the null-terminator
    uint16_t firstArgSize = (uint16_t)buf1[1];
    uint16_t secondArgSize = (uint16_t)buf1[3];

    //Receives the arguments and Allocate the arguments according to their size
    msg->firstArg = NULL;
    if(firstArgSize > 0)
    {
        msg->firstArg = (char*) malloc(firstArgSize * sizeof(char));
        n_bytes = recv(sockFD, (void*) msg->firstArg, firstArgSize * sizeof(char), 0);
        msg->firstArg[firstArgSize-1] = '\0';
        if(n_bytes <= 0)
            return (int)n_bytes;
    }

    msg->secondArg = NULL;
    if(secondArgSize > 0)
    {
        msg->secondArg = (char*) malloc(secondArgSize * sizeof(char));
        n_bytes = recv(sockFD, (void*) msg->secondArg, secondArgSize * sizeof(char), 0);
        msg->secondArg[secondArgSize-1] = '\0';
        if(n_bytes <= 0)
            return (int)n_bytes;
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
        return (int)n_bytes;
        
    if(msg.firstArg != NULL)
    {
        n_bytes = send(sockFD, msg.firstArg, firstArgSize*sizeof(char), 0);
        if(n_bytes <= 0)
            return (int)n_bytes;
    }
    if(msg.secondArg != NULL)
    {
        n_bytes = send(sockFD, msg.secondArg, secondArgSize*sizeof(char), 0);
        if(n_bytes <= 0)
            return (int)n_bytes;
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

/**
 * @brief  Write the data in the message to a buffer
 * @note   
 * @param  message: Read message
 * @param  buffer: The buffer that will be written to
 * @retval None
 */
void serialize_auth_message(AuthMessage* message, char* buffer)
{
    buffer[0] = (char)message->messageID;
    // Copy the strings to the buffer positions
    memcpy(&buffer[1], message->firstArg, 100);
    memcpy(&buffer[101], message->secondArg, 100);

    //Make sure they are null-terminated strings (even if it cuts the data)
    buffer[100] = '\0';
    buffer[200] = '\0';
}

/**
 * @brief  Reads the data in a buffer and converts it into a message
 * @note   
 * @param  message: Where the message will be written
 * @param  buffer: Read buffer
 * @retval None
 */
void deserialize_auth_message(AuthMessage* message, char* buffer)
{
    message->messageID = (int8_t)buffer[0];
    //Copy string from buffer to local variables (\0 added to prevent a bad string crashing the server)
    memcpy(message->firstArg, &buffer[1], 100);
    message->firstArg[100] = '\0';
    memcpy(message->secondArg, &buffer[101], 100);
    message->secondArg[200] = '\0';
}

/**
 * @brief  Creates an AuthMessage struct, and copies the args strings to the struct
 * @retval the new AuthMessage struct instance
 */
AuthMessage create_auth_message(const int8_t message_id, const char* first_arg, const char* second_arg)
{
    AuthMessage msg = { .messageID = message_id };

    //Copies the strings to the message structure.
    //strncat is used because it makes sure no more the the size of the string will be written
    if(first_arg != NULL)
    {
        msg.firstArg[0] = '\0';
        strncat(msg.firstArg, first_arg, sizeof(msg.firstArg)-1);
    }
    if(second_arg != NULL)
    {
        msg.secondArg[0] = '\0';
        strncat(msg.secondArg, second_arg, sizeof(msg.secondArg)-1);
    }

    return msg;
}

/**
 * @brief Sends a message on the with the contents of the message (uses serialize_auth_message)
 * 
 * @param msg 
 * @return int 1 means it was sent sucessfully, other values means an error ocurred
 */
int send_auth_message(AuthMessage msg, int sock, struct sockaddr_in server_address)
{
    char buf[AUTH_MSG_BUFFER_SIZE];

    //Convert the message to the buffer
    serialize_auth_message(&msg, buf);

    // Send the message
    ssize_t send_status = sendto(sock, buf, AUTH_MSG_BUFFER_SIZE, 0, (struct sockaddr*)&server_address, sizeof(server_address));
    if(send_status != AUTH_MSG_BUFFER_SIZE)
    {
        //Didn't send correctly, send the send_status, except if 0>send_status>AUTH_MSG_BUFFER_SIZE, then send -1
        return send_status > 0 ? -1 : (int)send_status;
    }

    return 1;
}