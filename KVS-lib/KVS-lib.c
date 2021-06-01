#include "KVS-lib.h"

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "../shared/hashtable.h"
#include "../shared/message.h"

HashTable CallbackTable;
int client_session_number;

int sock;
int sock_callback;
pthread_t thread_sock_callback;

struct sockaddr_un server_address;
struct sockaddr_un server_callback_address;

struct sockaddr_un sock_addr;
struct sockaddr_un sock_callback_addr;

//This is a random str that the server sends when login is successful.
//It is used when we want to connect to the callback socket, so that the
// server identifies it's us.
char session_id[SESSION_ID_STR_SIZE];

void* receive_callback_routine(void* in);

int establish_connection(const char *group_id, const char *secret)
{
    sock_callback = 0;

    char socket_addr[100];
    sprintf(socket_addr, "/tmp/client_sock_%d", getpid());
    remove(socket_addr);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return(ERROR_FAILED_CONNECTING);
    }

    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, socket_addr);

    if (bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
    {
        return(ERROR_FAILED_CONNECTING);
    }

    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SERVER_ADDRESS);

    //Connects to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)))
    {
        return(ERROR_FAILED_CONNECTING);
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

    // If login successful
    if (msg.messageID == MSG_OKAY)
    {
        // Create table for callback keys/functions
        CallbackTable = table_create(NULL);

        // Get the session ID
        strcpy(session_id, msg.firstArg);

        //Send the PID to the server
        pid_t pid = getpid();
        if(send(sock, &pid, sizeof(pid_t), 0) < (ssize_t)sizeof(pid))
        {
            return(ERROR_SENDING);
        }

        return (0);
    }
    else
    {
        close(sock);
        sock = 0;
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
    //Cirar o socket se ele ainda nao tier sido criado e conectar ao servidor
    if (sock_callback == 0)
    {
        char socket_addr[100];
        sprintf(socket_addr, "/tmp/client_sock_%d_callback", getpid());
        remove(socket_addr);

        sock_callback = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_callback == -1)
        {
            sock_callback = 0;
            return(ERROR_FAILED_CONNECTING);
        }

        sock_callback_addr.sun_family = AF_UNIX;
        strcpy(sock_callback_addr.sun_path, socket_addr);

        if (bind(sock_callback, (struct sockaddr *)&sock_callback_addr, sizeof(sock_callback_addr)) == -1)
        {
            close(sock_callback);
            sock_callback = 0;
            return(ERROR_FAILED_CONNECTING);
        }

        
        server_callback_address.sun_family = AF_UNIX;
        strcpy(server_callback_address.sun_path, SERVER_CALLBACK_ADDRESS);

        //Connects to the server
        if (connect(sock_callback, (struct sockaddr *)&server_callback_address, sizeof(server_callback_address)))
        {
            close(sock_callback);
            sock_callback = 0;
            return(ERROR_FAILED_CONNECTING);
        }

        //Send session ID to server
        send(sock_callback, session_id, SESSION_ID_STR_SIZE, 0);

        uint8_t ok = 0;
        //Receive confirmation
        recv(sock_callback, &ok, sizeof(ok), 0);
            
        if(ok != 1)
        {
            close(sock_callback);
            sock_callback = 0;
            return(ERROR_FAILED_CONNECTING);
        }

        //Abrir um thread, que vai ter uma rotina que só corre receive_message.
        pthread_create(&thread_sock_callback, NULL, receive_callback_routine, NULL);
    }

    msg.messageID = MSG_REGISTER_CALLBACK;
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
    
    if (msg.messageID == MSG_OKAY)
    {
        //Inserts the key and callback function in the table
        table_insert(&CallbackTable, key, (void *)callback_function);
        return (1);
    }
    else
    {
        return (msg.messageID);
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
        sock = 0;

        if(sock_callback != 0)
        {
            close(sock_callback);
            pthread_join(thread_sock_callback, NULL);
        }
        sock_callback = 0;

        

        //Free the callback table
        table_free(&CallbackTable);

        return 1;
    }

    //TODO close sock_callback if opened

    return -1;
}

void* receive_callback_routine(__attribute__((unused)) void* in)
{
    Message msg;

    while(1)
    {
        if(receive_message(sock_callback, &msg) != 1)
        {
            printf("Receive from callback socket failed\n");
            return NULL;
        }

        if(msg.messageID == MSG_CALLBACK)
        {
            printf("Debug: recebido callback de %s\n", msg.firstArg);
            void (*callback_function)(char *) = table_get(&CallbackTable, msg.firstArg);
            callback_function(msg.firstArg);
        }
    }
}
