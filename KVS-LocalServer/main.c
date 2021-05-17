#include "hashtable.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "../shared/message.h"

#define SERVER_ADDRESS "/tmp/server"




typedef struct Client_struct
{
    int sockFD;
    pthread_t thread;

    struct Client_struct* next;
} Client;
Client* connected_clients_list = NULL;

/*void* buffer = convert_message_to_buffer(message);

send(buffer, sizeof(buffer));


buffer[0] = message->tipoDeMensagem;*/

int main()
{
    HashTable table = table_create();
    table_insert(&table, "miguel", "fixe");
    table_insert(&table, "ab", "ola1");
    table_insert(&table, "ba", "ola2");

    printf("miguel -> %s\n", (char *)table_get(&table, "miguel"));
    printf("ab -> %s\n", (char *)table_get(&table, "ab"));
    printf("ba -> %s\n", (char *)table_get(&table, "ba"));

    table_delete(&table, (void *)"ab");

    //DEvia dar um apontador para NULL, pq removemos o ab
    printf("ab -> pointer: %p\n", table_get(&table, "ab"));
    printf("ab -> pointer: %p\n", table_get(&table, "bbbb"));

    /*MessageHeader header;
    recv(header, sizeof(MessageHeader))

    char* key;
    recv(key, tamanhoDoPrimeiroArg * sizeof(char))

    if()
    {
        char* value;
        recv(key, tamanhoDoPrimeiroArg * sizeof(char))
    }*/
}

void* thread_client_routine(void* in)
{
    Client* client = (Client*) in;
    int socketFD = client->sockFD;

    char* str = "teste";
    send(socketFD, str, sizeof(str), 0);

    while(1)
    {
        recv();
    }

    close(socketFD);
}

void client_connected(int clientFD)
{
    //Add client to the list of connected clients (connected_clients_list)
    Client* client = (Client*)malloc(sizeof(Client));
    client->next = NULL;
    client->sockFD = clientFD;

    if(connected_clients_list == NULL)
    {
        //The list was empty
        connected_clients_list = client;
    }
    else
    {
        //Adds the client to the end of the list
        Client* aux = connected_clients_list;

        while(aux != NULL && aux->next != NULL)
            aux = aux->next;

        aux->next = client;
    }

    //Starts the client thread, and saves the thread handler in client->thread
    pthread_create(&client->thread, NULL, thread_client_routine, (void*)client);
}

void create_server()
{
    int listen_sock;
    struct sockaddr_un listen_sock_addr;

    //Removes the previous socket file
    remove(SERVER_ADDRESS);

    //Creates the socket
    listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        exit(-1);
    }
    printf("socket created\n");

    //Sets the socket address
    listen_sock_addr.sun_family = AF_UNIX;
    strcpy(listen_sock_addr.sun_path, SERVER_ADDRESS);

    //Binds the socket to the listen address
    if (bind(listen_sock, (struct sockaddr *)&listen_sock_addr, sizeof(listen_sock_addr)) == -1)
    {
        exit(-1);
    }
    printf("Socket binded with an address %s\n", SERVER_ADDRESS);

    //Listen on that socket for incoming connections
    if (listen(listen_sock, 5) == -1)
    {
        printf("Error listening\n");
        printf("Error: %s\n", strerror(errno));
        exit(-1);
    }

    while (1)
    {
        //Waits and accepts client connections
        int clientFD = accept(listen_sock, NULL, NULL);


        if (clientFD < 0)
        {
            printf("Erro accepting client\n");
            printf("Error: %s\n", strerror(errno));
            exit(-1);
        }
        else
        {
            printf("A new client has connected\n");

            client_connected(clientFD);
        }
    }
}