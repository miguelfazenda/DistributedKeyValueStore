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
    msg.firstArg = (char*) group_id;
    msg.secondArg = (char*) secret;
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

    printf("put_value: key: %s\n", msg.firstArg);
    printf("put_value: value: %s\n", msg.secondArg);

    if(send_message(sock, msg) == -1)
    {
        return(-1); //to do: erros
    }

    //Receive response from server
    if(receive_message(sock, &msg) == -1 )
    {
        return(-1); // to do: erros
    }
    
    if(msg.messageID == MSG_OKAY)
    {
        return(1);
    }
    
    //If it wasn't MSG_OKAY, then return the error contained in msg.messageID
    return(msg.messageID);
}

// Allocates memory to store the value, 
int get_value(char* key, char** value)
{
    Message msg;
    msg.messageID = MSG_GET;
    msg.firstArg = key;
    msg.secondArg = NULL;

    if(send_message(sock, msg) == -1)
    {
        return(-1); //to do: erros
    }

    //Receive response from server
    if(receive_message(sock, &msg) == -1 )
    {
        return(-1); // to do: erros
    }

    if(msg.messageID == MSG_OKAY)
    {
        printf("Value retrieved: %s\n", msg.secondArg);
        *value = (char*) malloc (sizeof(msg.secondArg));
        strcpy(*value, msg.secondArg);
        return(1);
    }

    //If it wasn't MSG_OKAY, then return the error contained in msg.messageID
    return(msg.messageID);
}

int delete_value(char* key)
{
    Message msg;
    msg.messageID = MSG_DELETE;
    msg.firstArg = key;
    msg.secondArg = NULL;

    if(send_message(sock, msg) == -1)
    {
        return(-1); //to do: erros
    }

    //Receive response from server
    if(receive_message(sock, &msg) == -1 )
    {
        return(-1); // to do: erros
    }

    if(msg.messageID == MSG_OKAY)
    {
        return(1);
    }

    //If it wasn't MSG_OKAY, then return the error contained in msg.messageID
    return(msg.messageID);
}

int close_connection()
{
    if(sock != 0)
    {
        //Warn server this client is disconnecting
        Message msg = { .messageID = MSG_DISCONNECT, .firstArg = NULL, .secondArg = NULL };
        if(send_message(sock, msg) != 1)
            //Failed sending message about the disconnection
            return ERROR_DISCONNECTION_WARNING_FAILED;

        //Close the socket
        close(sock);

        return 1;
    }

    return -1;
}