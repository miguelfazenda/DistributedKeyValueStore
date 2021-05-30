#include "message_handling.h"
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "auth.h"

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

    //Check secret with AuthServer
    int8_t login_response = auth_send_login(client->group_id, msg->secondArg);

    if(login_response < 0)
    {
        printf("Error login message to auth server\n");
    }
    
    bool login_success = (login_response == 1);

    if(login_success)
    {
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
    }
    
    int8_t response_message_id = MSG_OKAY;

    if(!login_success)
    {
        //Failed login: wrong secret (login_response == 0) or error contained in login_response(such as ERROR_AUTH_GROUP_NOT_PRESENT)
        response_message_id = (login_response == 0) ? ERROR_WRONG_SECRET : login_response;
    }

    //Create and send message to client
    Message msg2;
    msg2.messageID = response_message_id;
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
int msg_received_put(Client *client, Message *msg)
{
    HashTable *group;

    //Finds the group table
    group = table_get(&groups_table, client->group_id);
    table_insert(group, msg->firstArg, strdup(msg->secondArg));

    //Create and send message to client
    Message msg2;
    msg2.messageID = MSG_OKAY;
    msg2.firstArg = NULL;
    msg2.secondArg = NULL;
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }
    // TO DO: ERROS -> return negative

    return (1); //success
}

int msg_received_get(Client *client, Message *msg)
{
    HashTable *group;
    char *value;
    Message msg2;

    //Finds the group table
    group = table_get(&groups_table, client->group_id);
    value = table_get(group, msg->firstArg);
    // If value == NULL, not found, else it is found and sent to client
    if (value == NULL)
    {
        msg2.messageID = -1;
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
        //to do: value not found
    }
    else
    {
        msg2.messageID = MSG_OKAY;
        msg2.firstArg = NULL;
        msg2.secondArg = value;
    }

    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }
    // TO DO: ERROS -> return negative

    return (1); //success
}

int msg_received_delete(Client *client, Message *msg)
{
    HashTable *group;
    Message msg2;

    //Finds the group table
    group = table_get(&groups_table, client->group_id);
    // If value == NULL, not found, else it is found and sent to client
    if (table_delete(group, msg->firstArg) == 0) //successfully deleted
    {
        msg2.messageID = MSG_OKAY;
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
        
    }
    else
    {
        // value not found
        msg2.messageID = -1; 
        msg2.firstArg = NULL;
        msg2.secondArg = NULL;
    }

    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}

