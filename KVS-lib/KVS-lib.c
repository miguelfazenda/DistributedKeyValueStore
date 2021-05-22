#include "../shared/message.h"
#include "KVS-lib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

int establish_connection (const char * group_id, const char * secret)
{
    struct sockaddr_un sock_addr;

    char SOCKET_ADDR[100];
    sprintf(SOCKET_ADDR, "/tmp/client_sock_%d", getpid());
    remove(SOCKET_ADDR);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        exit(-1);
    }

    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, SOCKET_ADDR);

    if(bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1){
        exit(-1);
    }

    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SERVER_ADDRESS);

    //Connects to the server
    if(connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)))
    {
        printf("connection with the server failed D:\n");
        exit(0);
    }


    //Está conectado
    Message msg;

    //Send the login message with group id and secret
    msg.messageID = MSG_LOGIN;
    msg.firstArg = group_id;
    msg.secondArg = secret;
    if(send_message(sock, msg) == -1)
    {
        return(-1); // to do: erros
    }
    
    /*char buf[100];
    recv(sock, buf, 100, 0);

    printf("buf: %s\n", buf);*/

    //Receive response from server
    if(receive_message(sock, &msg) == -1 )
    {
        return(-1); // to do: erros
    }
    
    if(msg.messageID == MSG_OKAY)
    {
        return(0);
    }
    else
    {
        return(msg.messageID);
    }
}

int put_value(char* key, char* value)
{
    Message msg;
    msg.messageID = MSG_PUT;
    msg.firstArg = key;
    msg.secondArg = value;

    if(send_message(sock, msg) == -1)
    {
        return(-1); //to do: erros
    }

    
}