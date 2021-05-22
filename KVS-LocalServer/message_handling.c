#include "message_handling.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>

 /*message_handling_functions = {
        NULL,
        msg_received_login,
        msg_received_put,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };*/

int msg_received_login(Client* client, Message* msg)
{
    //Copy the group id from the message argument
    char* group_id = (char*) malloc((strlen(msg->firstArg)+1) * sizeof(char));
    strcpy(group_id, msg->firstArg);

    //Save the group ID in the client struct
    client->group_id = group_id;

    // Check if table exists
    HashTable* table_for_group = (HashTable*) table_get(&groups_table, client->group_id);

    //If table does not exist, create one
    if(table_for_group == NULL)
    {
        table_for_group = (HashTable*) malloc(sizeof(HashTable));
        *table_for_group = table_create(free_value_str);
        table_insert(&groups_table, group_id, table_for_group);

        printf("Created a new table for group %s\n", group_id);
    }

    //Create and send message to client
    Message msg2;
    msg2.messageID = MSG_OKAY;
    msg2.firstArg = NULL;
    msg2.secondArg = NULL;
    if(send_message(client->sockFD, msg2) == -1)
    {
        return(-1);
    }

    /*if(wrong_secret)
    {
        client->stay_connected = 0;
    }*/
    
    return(1);
}

int msg_received_put(Client* client, Message* msg)
{
    HashTable* table;

    table_for_group = table_get(&groups_table, client->group_id);



    return 1;
}