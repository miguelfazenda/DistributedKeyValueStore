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

int msg_received_login(Client *client, Message *msg)
{
    //Copy the group id from the message argument
    char *group_id = (char *)malloc((strlen(msg->firstArg) + 1) * sizeof(char));
    strcpy(group_id, msg->firstArg);

    //Save the group ID in the client struct
    client->group_id = group_id;

    // Check if table exists
    HashTable *table_for_group = (HashTable *)table_get(&groups_table, client->group_id);

    //If table does not exist, create one
    if (table_for_group == NULL)
    {
        table_for_group = (HashTable *)malloc(sizeof(HashTable));
        *table_for_group = table_create(free_value_str);
        table_insert(&groups_table, group_id, table_for_group);

        printf("Created a new table for group %s\n", group_id);
    }

    //Create and send message to client
    Message msg2;
    msg2.messageID = MSG_OKAY;
    msg2.firstArg = NULL;
    msg2.secondArg = NULL;
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    /*if(wrong_secret)
    {
        client->stay_connected = 0;
    }*/

    return (1);
}
/*Message is received, this function assigns the value to the key and sends feedback on the execution to the application*/
//Returns -1 if failed sending response, 1 if success
int msg_received_put(Client *client, Message *msg)
{
    HashTable *group;

    //Finds the group table
    group = table_get(&groups_table, client->group_id);
    table_insert(group, msg->firstArg, msg->secondArg);

    //Create and send message to client
    Message msg2 = { .messageID = MSG_OKAY, .firstArg = NULL, .secondArg = NULL};
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}

int msg_received_get(Client *client, Message *msg)
{
    HashTable *group;
    char *value;
    Message msg2;

    //Finds the group table
    group = table_get(&groups_table, client->group_id);
    
    //Get the value
    value = table_get(group, msg->firstArg);
    
    // If value == NULL, not found, else it is found and sent to client
    if (value == NULL)
    {
        //Value not found
        msg2.messageID = -1;
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
        //to do: value not found
    }
    else
    {
        //Success getting the value
        msg2.messageID = MSG_OKAY;
        msg2.firstArg = NULL;
        msg2.secondArg = value;
    }

    //Send response to the client
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}

int msg_received_delete(Client *client, Message *msg)
{
    HashTable *group;
    Message msg2;

    //Finds the group table
    group = table_get(&groups_table, client->group_id);
    // If value == NULL, not found, else it is found and sent to client
    if (table_delete(group, msg->firstArg) == 0) 
    {
        //successfully deleted
        msg2.messageID = MSG_OKAY;
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
    }
    else
    {
        //Error deleting
        msg2.messageID = -1; //to do:erros (but key/value does not exist)
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
    }

    //Send response to the client
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}

