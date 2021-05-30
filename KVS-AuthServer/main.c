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

#define AUTH_SERVER_ADDRESS "/tmp/auth_server"
#define MSG_AUTH_CHECK_LOGIN 1

HashTable groups_table;
struct sockaddr_un sock_addr;
int sock;

#define RECV_BUF_SIZE 100
#define SEND_BUF_SIZE 100

void create_server();

int main()
{
    groups_table = table_create(free_value_hashtable);

    create_server();

    struct sockaddr_un sender_sock_addr;
    char recv_buf[RECV_BUF_SIZE];
    int n_bytes;

    memset(&sender_sock_addr, 0, sizeof(struct sockaddr_un));
    while(1) {
        socklen_t sender_sock_addr_size = sizeof(struct sockaddr_un);
        n_bytes = recvfrom(sock, &recv_buf, RECV_BUF_SIZE, 0,
                        (struct sockaddr*)&sender_sock_addr, &sender_sock_addr_size);

        __uint8_t messageId = (__uint8_t) recv_buf[0];

        if(messageId == MSG_AUTH_CHECK_LOGIN)
        {
            printf("Check login status\n");

            char send_buf[SEND_BUF_SIZE];

            sendto(sock, &send_buf, SEND_BUF_SIZE, 0, (struct sockaddr*)&sender_sock_addr, sender_sock_addr_size);
        }
        


        //sendto(sock, &primo, sizeof(int), 0, (struct sockaddr*)&sender_sock_addr, sender_sock_addr_size);
    }

    return 0;
}


void create_server()
{
    remove(AUTH_SERVER_ADDRESS);

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1){
        exit(-1);
    }
    printf("socket created\n");
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, AUTH_SERVER_ADDRESS);

    if(bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1){
        exit(-1);
    }
    printf("socket with an address %s\n", AUTH_SERVER_ADDRESS);
}