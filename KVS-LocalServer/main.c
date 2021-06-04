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
#include "../shared/auth_defines.h"
#include "client_list.h"
#include "message_handling.h"
#include "globals.h"
#include "auth.h"

#define SERVER_ADDRESS "/tmp/server"
#define SERVER_CALLBACK_ADDRESS "/tmp/server_callback"

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
void read_terminal(char *word);
const char *get_error_code_string(int8_t code, const char *generic_error);
int count_pairs();

//TODO criar doc para

int main(void)
{
    char operation[100], first_arg[100], second_arg[100];
    groups_table = table_create(free_value_hashtable);
    /*table_insert(&groups_table, "miguel", "fixe");
    table_insert(&groups_table, "ab", "ola1");
    table_insert(&groups_table, "ba", "ola2");

    printf("miguel -> %s\n", (char *)table_get(&groups_table, "miguel"));
    printf("ab -> %s\n", (char *)table_get(&groups_table, "ab"));
    printf("ba -> %s\n", (char *)table_get(&groups_table, "ba"));

    table_delete(&groups_table, (void *)"ab");

    //DEvia dar um apontador para NULL, pq removemos o ab
    printf("ab -> pointer: %p\n", table_get(&groups_table, "ab"));
    printf("ab -> pointer: %p\n", table_get(&groups_table, "bbbb"));*/

    if (auth_create_socket("127.0.0.1", 25565) != 1)
    {
        printf("Error creating the auth connection socket\n");
    }

    //TODO - remove this, Cria o grupo para testar
    int status_create_group = auth_create_group("a", "b");
    if (status_create_group == 1)
        printf("Grupo criado com sucesso\n");
    else
        printf("Erro a criar grupo: %d\n", status_create_group);

    char group_secret[AUTH_MESSAGE_STRUCT_ARG_SIZE];
    int status_get_secret = auth_get_secret("a", group_secret);
    if (status_get_secret == 1)
        printf("Segredo obtido: %s\n", group_secret);
    else if (status_get_secret == ERROR_AUTH_GROUP_NOT_PRESENT)
        printf("Erro grupo nao existente\n");
    else
        printf("Erro a obter segredo: %d\n", status_create_group);

    //Creates a thread to run the server accepting connections
    pthread_create(&server_thread, NULL, run_server, NULL);

    //Creates a thread to run the server accepting connections
    pthread_create(&server_thread_callbacks, NULL, run_callback_sock_accept, NULL);

    //Terminal reading
    printf("Welcome! Here are the commands for your operations:\n____________________\n");
    printf("create group: 'create'\ndelete group:'delete'\nshow group info:'show group'\nquit\n");
    printf("show application status: 'show app'\n");
    printf("____________________\n\n");

    while (!quitting)
    {
        int error;

        // Read operation from terminal
        printf("Enter operation: \n");
        read_terminal(operation);

        if (strcmp(operation, "create") == 0)
        {
            char group_id[1000];
            char group_secret[SECRET_SIZE];

            // Read group ID
            printf("Insert group ID you want to create:\n");
            read_terminal(group_id);

            // Generate random secret
            if (error = auth_create_group(group_id, group_secret))
            {
                printf("Group created successfully!\nGroup: %s\nSecret: %s\n", group_id, group_secret);
            }
            else
            {
                //TODO criar doc para o get_error
                printf("Error creating group: %s\n", get_error_code_string(error, "error"));
            }
        }
        else if (strcmp(operation, "delete") == 0)
        {
        }
        else if (strcmp(operation, "show group") == 0)
        {
            char group_id[1000];
            char group_secret[SECRET_SIZE];

            // Read group ID
            printf("Insert group ID to know information:\n");
            read_terminal(group_id);

            if (error = auth_get_secret(group_id, group_secret))
            {
                printf("Secret: %d\nNumbers of pairs key/value:%d\n", group_secret, count_pairs);
            }
            else
            {
                printf("Error creating group: %s\n", get_error_code_string(error, "error"));
            }
        }
        else if (strcmp(operation, "show app") == 0)
        {
            
        }
        else if (strcmp(operation, "quit") == 0)
        {
            quit();
            break;
        }
    }

    //We couldn't shutdown the listen_sock running accept,
    // Therefore we will just exit the program
    //Wait for server thread to stop
    //pthread_join(server_thread, NULL);

    //TODO table_free(groups_table);
    //TODO pthread_mutex_lock(&clients_with_callback_by_key_mtx); table_free(clients_with_callback_by_key); pthread_mutex_unlock(&clients_with_callback_by_key_mtx);

    return 1;
}

void read_terminal(char *word)
{
    while (fscanf(stdin, "%s", word) != 1)
    {
        printf("Invalid input. Insert again");
    }
}

void create_group_l()
{
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
    }
    else
    {
        printf("Client should have sent a login message!\n");
        client->stay_connected = 0;
    }

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

            if (msg.messageID == MSG_PUT)
            {
                if (msg_received_put(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }
            else if (msg.messageID == MSG_GET)
            {
                if (msg_received_get(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }
            else if (msg.messageID == MSG_DELETE)
            {
                if (msg_received_delete(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }
            else if (msg.messageID == MSG_REGISTER_CALLBACK)
            {
                if (msg_received_register_callback(client, &msg) == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }
            else if (msg.messageID == MSG_DISCONNECT)
            {
                //The client informed the server it is disconnecting.
                printf("Client has disconnected\n");
                client->stay_connected = 0;
            }

            /*if(msg.messageID < 0 || msg.messageID > MAX_HANDLING_FUNCTION_ID || message_handling_functions[msg.messageID] == NULL)
            {
                printf("Cant find message handling function for msg id: %d", msg.messageID);
            }
            else
            {
                //Call the function to handle the message
                int ret = message_handling_functions[msg.messageID](client, &msg);
                if(ret == -1)
                {
                    // Error, disconnect client
                    printf("Error receiving message from client. Disconnecting client\n");
                    client->stay_connected = 0;
                }
            }*/
        }
        else if (recv_status == 0)
        {
            //The socket has been shutdown (by quit())
            //TODO inform the client the server has shutdown
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
    //client_list_remove_and_free(&connected_clients, client);

    printf("Client thread has ended. Connection time %g seconds\n", difftime(client->time_disconnected, client->time_connected));

    return (NULL);
}

void client_connected(int clientFD)
{
    //Add client to the list of connected clients (connected_clients_list)
    Client *client = (Client *)malloc(sizeof(Client));
    client->next = NULL;
    client->sockFD = clientFD;
    client->group_id = NULL;
    client->stay_connected = true;
    client->connected = true;
    client->time_connected = time(NULL);
    client->time_disconnected = 0;
    client->callback_sock_fd = 0;

    client_list_add(&connected_clients, client);

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

            //Waits for the client to send it's session_id.
            //TODO put this in a thread, and also set a timeout for this recv
            char session_id[SESSION_ID_STR_SIZE];
            recv(client_callback_sock_fd, session_id, SESSION_ID_STR_SIZE, 0);

            uint8_t client_with_session_id_found = 0;

            //Find which client corresponds to this socket
            //Lock mutex
            //TODO ver se realmente é preciso Lock mutex, porque aqui so lemos a lista. No max é o client->next que muda, porque nao removemos nada da lista??
            pthread_mutex_lock(&connected_clients.mtx_client_list);

            Client *client = connected_clients.client_list;
            while (client != NULL)
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
            //TODO ver se realmente é preciso Lock mutex, porque aqui so lemos a lista. No max é o client->next que muda, porque nao removemos nada da lista??
            pthread_mutex_unlock(&connected_clients.mtx_client_list);

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
    pthread_mutex_lock(&connected_clients.mtx_client_list);
    Client *client = connected_clients.client_list;
    while (client != NULL)
    {
        //TODO avisar clientes que o servidor vai desligar. Talvez precise de um mutex, para nao enviar 2 coisas ao mesmo tempo com a outra thread
        //Na verdade avisar provavlmente nao faz sentido??

        //Shutdown the socket and wait for the thread to join
        shutdown(client->sockFD, SHUT_RDWR);
        pthread_join(client->thread, NULL);
    }
    pthread_mutex_unlock(&connected_clients.mtx_client_list);

    //Close listen_sock
    /*shutdown(listen_sock, SHUT_RDWR);
    close(listen_sock);
    listen_sock = 0;
    //Wait for server thread to stop
    pthread_join(server_thread, NULL);*/

    //We couldn't shutdown the listen_sock running accept,
    // Therefore we will just exit the program
}

/**
 * @brief  Translates an error code into a string 
 * @param  code: The error code
 * @param  generic_error: If there is no string for such error code, returns this string
 */
const char *get_error_code_string(int8_t code, const char *generic_error)
{
    if (code == ERROR_WRONG_SECRET)
        return "Wrong secret!";
    else if (code == ERROR_AUTH_GROUP_NOT_PRESENT)
        return "Group not present in the server!";
    else if (code == ERROR_FAILED_AUTHENTICATION)
        return "Failed authentication.";
    else if (code == ERROR_SENDING)
        return "Failed sending information to server.";
    else if (code == ERROR_RECEIVING)
        return "Failed receiving information from server.";
    else if (code == ERROR_VALUE_NOT_FOUND)
        return "Value not found.";

    return generic_error;
}

int count_pairs()
{
    int count = 0;
    TableItem* item;

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        item = groups_table.array[i];
        while(item != NULL)
        {
            count++;
            item = (item)->next;
        }
    }

    return(count);
}