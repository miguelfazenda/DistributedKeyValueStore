#include "message.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>

Message receive_message(int sockFD)
{
    Message msg;

    //Receives the message header, that contains the ID and size of the arguments
    unsigned char buf1[5];
    recv(sockFD, buf1, 5, 0);

    msg.messageID = buf1[0];

    __uint16_t firstArgSize = (__uint16_t)buf1[1];
    __uint16_t secondArgSize = (__uint16_t)buf1[3];

    //Allocate the arguments according to their size
    msg.firstArg = (char*) malloc(firstArgSize * sizeof(char));
    msg.secondArg = (char*) malloc(secondArgSize * sizeof(char));

    //Receives the arguments
    recv(sockFD, msg.firstArg, firstArgSize * sizeof(char), 0);
    recv(sockFD, msg.secondArg, secondArgSize * sizeof(char), 0);

    return msg;
}

void send_message(int sockFD, Message msg)
{
    char buf1[5]; // porque é que o buf tem tamanho 5?
    buf1[0] = msg.messageID;
    size_t firstArgSize = strlen(msg.firstArg)+1;
    size_t secondArgSize = strlen(msg.secondArg)+1;

    buf1[1] = firstArgSize;
    buf1[3] = secondArgSize; //porque é que este buf é na posição 3?

    send(sockFD, buf1, 5, 0);
    send(sockFD, msg.firstArg, firstArgSize*sizeof(char), 0);
    send(sockFD, msg.secondArg, secondArgSize*sizeof(char), 0);
}