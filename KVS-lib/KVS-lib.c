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

int establish_connection(const char *group_id, const char *secret)
{
    struct sockaddr_un sock_addr;

    char SOCKET_ADDR[100];
    sprintf(SOCKET_ADDR, "/tmp/client_sock_%d", getpid());
    remove(SOCKET_ADDR);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        exit(ERROR_FAILED_CONNECTING);
    }

    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, SOCKET_ADDR);

    if (bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
    {
        exit(ERROR_FAILED_CONNECTING);
    }

    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SERVER_ADDRESS);

    //Connects to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)))
    {
        exit(ERROR_FAILED_CONNECTING);
    }

    //Está conectado
    Message msg;

    //Send the login message with group id and secret
    msg.messageID = MSG_LOGIN;
    msg.firstArg = (char *)group_id;
    msg.secondArg = (char *)secret;
    if (send_message(sock, msg) == -1)
    {
        return (ERROR_SENDING);
    }

    //Receive response from server
    if (receive_message(sock, &msg) == -1)
    {
        return (ERROR_RECEIVING);
    }

    if (msg.messageID == MSG_OKAY)
    {
        return (0);

        // Create table for callback keys/functions
        CallbackTable = table_create(NULL);
    }
    else
    {
        return (msg.messageID);
    }
}

int put_value(char *key, char *value)
{
    Message msg;
    msg.messageID = MSG_PUT;
    msg.firstArg = key;
    msg.secondArg = value;

    printf("put_value: key: %s\n", msg.firstArg);
    printf("put_value: value: %s\n", msg.secondArg);

    //Send message to server
    if (send_message(sock, msg) == -1)
    {
        return (ERROR_SENDING);
    }

    //Receive response from server
    if (receive_message(sock, &msg) == -1)
    {
        return (ERROR_RECEIVING);
    }

    if (msg.messageID == MSG_OKAY)
    {
        return (1);
    }

    //If it wasn't MSG_OKAY, then return the error contained in msg.messageID
    return (msg.messageID);
}

// Allocates memory to store the value,
int get_value(char *key, char **value)
{
    Message msg;
    msg.messageID = MSG_GET;
    msg.firstArg = key;
    msg.secondArg = NULL;

    if (send_message(sock, msg) == -1)
    {
        return (ERROR_SENDING); //to do: erros
    }

    //Receive response from server
    if (receive_message(sock, &msg) == -1)
    {
        return (ERROR_RECEIVING); // to do: erros
    }

    if (msg.messageID == MSG_OKAY)
    {
        printf("Value retrieved: %s\n", msg.secondArg);
        *value = (char *)malloc(sizeof(msg.secondArg));
        strcpy(*value, msg.secondArg);
        return (1);
    }

    //If it wasn't MSG_OKAY, then return the error contained in msg.messageID
    return (msg.messageID);
}

int delete_value(char *key)
{
    Message msg;
    msg.messageID = MSG_DELETE;
    msg.firstArg = key;
    msg.secondArg = NULL;

    if (send_message(sock, msg) == -1)
    {
        return (ERROR_SENDING);
    }

    //Receive response from server
    if (receive_message(sock, &msg) == -1)
    {
        return (ERROR_RECEIVING);
    }

    if (msg.messageID == MSG_OKAY)
    {
        return (1);
    }

    //If it wasn't MSG_OKAY, then return the error contained in msg.messageID
    return (msg.messageID);
}

int register_callback(char *key, void (*callback_function)(char *))
{
    Message msg;
    msg.messageID = MSG_CALLBACK;
    msg.firstArg = key;
    msg.secondArg = NULL;

    // Send message to server
    if (send_message(sock, msg) == -1)
    {
        return (ERROR_SENDING);
    }

    //Receive response from server
    if (receive_message(sock, &msg) == -1)
    {
        return (ERROR_RECEIVING);
    }

    //Inserts the key and callback function in the table
    if (msg.messageID == MSG_OKAY)
    {
        table_insert(&CallbackTable, key, callback_function);
        return(1);
    }
    else
    {
        return(msg.messageID);
    }
}

int close_connection()
{
    if (sock != 0)
    {
        //Warn server this client is disconnecting
        Message msg = {.messageID = MSG_DISCONNECT, .firstArg = NULL, .secondArg = NULL};
        if (send_message(sock, msg) != 1)
            //Failed sending message about the disconnection
            return ERROR_DISCONNECTION_WARNING_FAILED;

        //Close the socket
        close(sock);

        //Free the callback table
        table_free(&CallbackTable);

        return 1;
    }

    return -1;
}