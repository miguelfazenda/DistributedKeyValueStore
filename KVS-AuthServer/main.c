#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "../shared/message.h"
#include "../shared/hashtable.h"

#define SERVER_ADDRESS "/tmp/server"

HashTable groups_table;

void create_server();

int main()
{
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

    create_server();

    return 0;
}

/**
 * @brief 
 * 
 * @param clientFD 
 * @return int Error code, or 1 for success
 */
int handle_client(int clientFD)
{
    Message msg;
    if(receive_message(clientFD, &msg) == -1)
        return -1;

    if(msg.messageID == MSG_LOGIN)
    {
        char* group_id = msg.firstArg;
        char* secret = msg.secondArg;

        //TODO Verificar se o secret está correto

        bool correct_login = (strcmp(secret, "segredo") == 0);

        Message msg2;
        msg2.messageID = MSG_OKAY;
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
    }

    free_message(&msg);

    return 1;
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
            printf("A client has connected\n");

            handle_client(clientFD);
            //close the connection
            close(clientFD);
        }
    }
}