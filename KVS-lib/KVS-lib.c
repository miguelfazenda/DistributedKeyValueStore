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

int establish_connection (char * group_id, char * secret)
{
    int recv_sock;
    struct sockaddr_un recv_sock_addr;

    char SOCKET_ADDR[100];
    sprintf(SOCKET_ADDR, "/tmp/client_sock_%d", getpid());
    remove(SOCKET_ADDR);

    recv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (recv_sock == -1)
    {
        exit(-1);
    }

    recv_sock_addr.sun_family = AF_UNIX;
    strcpy(recv_sock_addr.sun_path, SOCKET_ADDR);

    if(bind(recv_sock, (struct sockaddr*)&recv_sock_addr, sizeof(recv_sock_addr)) == -1){
        exit(-1);
    }

    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SERVER_ADDRESS);

    //Connects to the server
    if(connect(recv_sock, (struct sockaddr*)&server_address, sizeof(server_address)))
    {
        printf("connection with the server failed D:\n");
        exit(0);
    }


    //Está conectado
    Message msg;
    msg.messageID = 5;
    msg.firstArg = group_id;
    msg.secondArg = secret;

    send_message(recv_sock, msg);
    
    char buf[100];
    recv(recv_sock, buf, 100, 0);

    printf("buf: %s", buf);

    
    return 0;
}