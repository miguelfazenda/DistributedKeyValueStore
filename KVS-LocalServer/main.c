#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include "../shared/hashtable.h"
#include "../shared/message.h"
#include "client_list.h"
#include "message_handling.h"
#include "globals.h"
#include "auth.h"
#include "ui.h"

#define SERVER_ADDRESS "/tmp/server"
#define SERVER_CALLBACK_ADDRESS "/tmp/server_callback"

#define AUTH_SERVER_PORT 25565

//Server thread
pthread_t server_thread;
//Server thread accepting callback sockets
pthread_t server_thread_callbacks;
//Socket listening to incoming connections
int listen_sock;
int listen_callback_sock;

bool quitting = false;

void *run_server(void *a);
void *run_callback_sock_accept(void *a);
void *thread_client_routine(void *in);
void client_connected(int clientFD);
void disconnect_client(Client *client);
void quit(void);
const char *get_error_code_string(int8_t code, const char *generic_error);

void free_client_list_item_list(void* ptr);

int main(int argc, char *argv[])
{
    //Init groups table and mutex
    groups_table = table_create(free_value_hashtable);
    pthread_mutex_init(&groups_table_mtx, NULL);

    //Init clients list and mutex
    clients.client_list = NULL;
    pthread_mutex_init(&clients.mtx_client_list, NULL);
    
    clients_with_callback_by_key = table_create(free_client_list_item_list);
    pthread_mutex_init(&clients_with_callback_by_key_mtx, NULL);

    const char* host_name = "127.0.0.1";

    if(argc > 1)
    {
        //Get the host_name from arguments
        host_name = argv[1];
    }

    printf("Connecting to AuthServer on %s:%d. Use \"LocalServer <AuthServerAddress>\" to connect to another address.\n", host_name, AUTH_SERVER_PORT);

    int8_t auth_conn_status = auth_create_socket(host_name, AUTH_SERVER_PORT);
    if (auth_conn_status != 1)
    {
        printf("Error creating the auth connection socket: %s\n", get_error_code_string(auth_conn_status, "error"));
        exit(-1);
    }

    srand((unsigned int)time(NULL));

    //Creates a thread to run the server accepting connections
    pthread_create(&server_thread, NULL, run_server, NULL);

    //Creates a thread to run the server accepting connections
    pthread_create(&server_thread_callbacks, NULL, run_callback_sock_accept, NULL);

    //UI
    run_ui();

    //After UI closed, quit the program
    quit();

    //We couldn't shutdown the listen_sock running accept,
    // Therefore we will just exit the program
    //Wait for server thread to stop
    //pthread_join(server_thread, NULL);

    table_free(&groups_table);
    table_free(&clients_with_callback_by_key);

    return 1;
}

void *thread_client_routine(void *in)
{
    Client *client = (Client *)in;
    int socketFD = client->sockFD;

    Message msg;
    int recv_status = receive_message(socketFD, &msg);

    if (msg.messageID == MSG_LOGIN)
    {
        if (msg_received_login(client, &msg) == -1)
        {
            // Error, disconnect client
            printf("Error receiving message from client. Disconnecting client\n");
            client->stay_connected = 0;
        }

        if(client->group_id == NULL)
            //Failed login
            client->stay_connected = 0;
    }
    else
    {
        printf("Client should have sent a login message!\n");
        client->stay_connected = 0;
    }

    free_message(&msg);

    //Receives the PID of the client
    if (recv(client->sockFD, &client->pid, sizeof(pid_t), 0) < (ssize_t)sizeof(pid_t))
    {
        // Error, disconnect client
        printf("Error receiving message from client. Disconnecting client\n");
        client->stay_connected = 0;
    }

    if (client->stay_connected)
    {
        //Login was successful
        printf("Client %p logged in successfuly. GroupID: %s, PID: %d\n", (void *)client, client->group_id, client->pid);
    }

    //While client hasn't disconnected, and login was successful
    while (client->stay_connected)
    {
        msg.firstArg = NULL;
        msg.secondArg = NULL;

        recv_status = receive_message(socketFD, &msg);
        if (recv_status == 1)
        {
            //Received message successfully
            printf("Received msg %d, %s, %s\n", msg.messageID, msg.firstArg, msg.secondArg);

            //Register the appropriate receive function for this messageID
            int (*receive_function)(Client*, Message*) = NULL;
            if (msg.messageID == MSG_PUT)
                receive_function = msg_received_put;
            else if (msg.messageID == MSG_GET)
                receive_function = msg_received_get;
            else if (msg.messageID == MSG_DELETE)
                receive_function = msg_received_delete;
            else if (msg.messageID == MSG_REGISTER_CALLBACK)
                receive_function = msg_received_register_callback;
            else if (msg.messageID == MSG_DISCONNECT)
            {
                //The client informed the server it is disconnecting.
                printf("Client has disconnected\n");
                client->stay_connected = 0;
            }
            else
            {
                printf("Cant find message handling function for msg id: %d\n", msg.messageID);
            }

            int status = 0;

            //Run the receive function appropriate for this messageID
            if (receive_function != NULL)
                status = receive_function(client, &msg);

            if(status == -1)
            {
                // Error, disconnect client
                printf("Error receiving message from client. Disconnecting client\n");
                client->stay_connected = 0;
            }            
        }
        else if (recv_status == 0)
        {
            //The socket has been shutdown (by quit())
            printf("Client socket has been shutdown\n");
            client->stay_connected = 0;
        }
        else
        {
            // Error, disconnect client
            printf("Error receiving message from client. Disconnecting client A\n");
            client->stay_connected = 0;
        }

        //Free the message
        free_message(&msg);
    }

    //Client disconnected, close socket and mark it as disconnected
    disconnect_client(client);
    //client_list_remove_and_free(&clients, client);

    printf("Client thread has ended. Connection time %g seconds\n", difftime(client->time_disconnected, client->time_connected));

    return (NULL);
}

void client_connected(int clientFD)
{
    //Add client to the list of connected clients (clients_list)
    Client* client = (Client*)malloc(sizeof(Client));
    client->next = NULL;
    client->sockFD = clientFD;
    client->group_id = NULL;
    client->stay_connected = true;
    client->connected = true;
    client->time_connected = time(NULL);
    client->time_disconnected = 0;
    client->callback_sock_fd = 0;

    client_list_add(&clients, client);

    //Starts the client thread, and saves the thread handler in client->thread
    pthread_create(&client->thread, NULL, thread_client_routine, (void *)client);
}

/**
 * @brief  Marks the client as disconnected, and closes the socket
 * @note   
 * @param  client: 
 * @retval None
 */
void disconnect_client(Client *client)
{
    client->time_disconnected = time(NULL);
    client->stay_connected = false;
    client->connected = false;

    //On closing the socket, the receive_message will fail, and
    close(client->sockFD);
    client->sockFD = 0;

    if (client->callback_sock_fd != 0)
        close(client->callback_sock_fd);


    //Removes this client from anywhere on the list of registered callbacks
    pthread_mutex_lock(&clients_with_callback_by_key_mtx);
    for(int i=0; i<TABLE_SIZE; i++)
    {
        TableItem* item = clients_with_callback_by_key.array[i];
        if(item == NULL)
            continue;

        Client_List_Item** c = (Client_List_Item**)(&item->value);
        while(*c != NULL)
        {
            if((*c)->client == client)
            {
                Client_List_Item* aux = *c;
                *c = (*c)->next;

                free(aux);
                continue;
            }

            c = &(*c)->next;
        }
    }
    pthread_mutex_unlock(&clients_with_callback_by_key_mtx);
}

void *run_server(__attribute__((unused)) void *a)
{
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
            printf("Erro accepting client: %s\n", strerror(errno));
            exit(-1);
        }
        else
        {
            printf("A new client has connected\n");

            client_connected(clientFD);
        }
    }

    //Close the socket
    if (listen_sock != 0)
        close(listen_sock);
    listen_sock = 0;

    return NULL;
}

void *run_callback_sock_accept(__attribute__((unused)) void *a)
{
    struct sockaddr_un listen_callback_sock_addr;

    //Removes the previous socket file
    remove(SERVER_CALLBACK_ADDRESS);

    //Creates the socket
    listen_callback_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_callback_sock == -1)
    {
        exit(-1);
    }
    printf("callback socket created\n");

    //Sets the socket address
    listen_callback_sock_addr.sun_family = AF_UNIX;
    strcpy(listen_callback_sock_addr.sun_path, SERVER_CALLBACK_ADDRESS);

    //Binds the socket to the listen address
    if (bind(listen_callback_sock, (struct sockaddr *)&listen_callback_sock_addr, sizeof(listen_callback_sock_addr)) == -1)
    {
        exit(-1);
    }
    printf("callback socket binded with an address %s\n", SERVER_CALLBACK_ADDRESS);

    //Listen on that socket for incoming connections
    if (listen(listen_callback_sock, 5) == -1)
    {
        printf("Error listening: %s\n", strerror(errno));
        exit(-1);
    }

    while (1)
    {
        //Waits and accepts client connections
        int client_callback_sock_fd = accept(listen_callback_sock, NULL, NULL);

        if (client_callback_sock_fd < 0)
        {
            printf("Erro accepting client callback socket: %s\n", strerror(errno));
            exit(-1);
        }
        else
        {
            printf("A new client callback socket has connected\n");

            //Set timeout for socket to receive session_id
            struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
            setsockopt(auth_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            //Waits for the client to send it's session_id. 
            char session_id[SESSION_ID_STR_SIZE];
            recv(client_callback_sock_fd, session_id, SESSION_ID_STR_SIZE, 0);

            uint8_t client_with_session_id_found = 0;

            //Find which client corresponds to this socket
            //Lock mutex
            pthread_mutex_lock(&clients.mtx_client_list);

            Client* client = clients.client_list;
            while(client != NULL)
            {
                if (client->connected)
                {
                    //Compare each client's session_id with the one we just received.
                    if (strcmp(session_id, client->session_id) == 0)
                    {
                        //Client with that session_id found!
                        client->callback_sock_fd = client_callback_sock_fd;
                        client_with_session_id_found = 1;
                        break;
                    }
                }

                client = client->next;
            }

            //Unlock mutex
            pthread_mutex_unlock(&clients.mtx_client_list);

            //Tell the client we received the session_id! Send 1 if we associated with the right client, 0 if we couldn't find
            send(client_callback_sock_fd, &client_with_session_id_found, sizeof(client_with_session_id_found), 0);
        }
    }

    //Close the socket
    if (listen_callback_sock != 0)
        close(listen_callback_sock);
    listen_callback_sock = 0;

    return NULL;
}

void quit(void)
{
    printf("Quitting\n");
    quitting = true;

    //Stop all client scokets and threads
    pthread_mutex_lock(&clients.mtx_client_list);
    Client* client = clients.client_list;
    while(client != NULL)
    {
        if(client->connected)
        {
            //Shutdown the socket and wait for the thread to join
            shutdown(client->sockFD, SHUT_RDWR);
            pthread_join(client->thread, NULL);
        }

        client = client->next;
    }
    pthread_mutex_unlock(&clients.mtx_client_list);

    //Close listen_sock
    /*shutdown(listen_sock, SHUT_RDWR);
    close(listen_sock);
    listen_sock = 0;
    //Wait for server thread to stop
    pthread_join(server_thread, NULL);*/

    //We couldn't shutdown the listen_sock running accept,
    // Therefore we will just exit the program
}

void free_client_list_item_list(void* ptr)
{
    //Get the first item
    Client_List_Item* item = (Client_List_Item*)ptr;
    while (item != NULL)
    {
        Client_List_Item* aux = item;

        item = item->next;

        free(aux);
    }
    
}