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

void generate_random_session_id(char *str);
void value_for_key_modified(char* key);

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
    char session_id[SESSION_ID_STR_SIZE];

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

    if(login_success)
    {
        //Login OK, send session_id
        generate_random_session_id(session_id);
        msg2.firstArg = session_id;
        
        strcpy(client->session_id, session_id);        
    }

    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    //TODO disconectar?
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

    //Message that will be sent to the client
    Message msg2 = { .messageID = -1, .firstArg = NULL, .secondArg = NULL };

    if(client->group_id == NULL)
    {
        //If client hasn't logged in yet
        msg2.messageID = ERROR_FAILED_AUTHENTICATION;
    }
    else
    {
        //Finds the group table
        group = table_get(&groups_table, client->group_id);

        //Check if the group table exists
        if(group == NULL)
        {
            //Creates a new table for that group
            group = (HashTable*) malloc(sizeof(HashTable));
            *group = table_create(free_value_str);
            table_insert(&groups_table, client->group_id, group);

            printf("Created a new table for group %s\n", client->group_id);
        }

        //Inserts the value in the table
        table_insert(group, msg->firstArg, strdup(msg->secondArg));
        value_for_key_modified(msg->firstArg);

        msg2.messageID = MSG_OKAY;
    }

    //Send message to client
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}

int msg_received_get(Client *client, Message *msg)
{
    HashTable *group;
    char *value = NULL;
    Message msg2 = { .messageID = -1, .firstArg = NULL, .secondArg = NULL };

    if(client->group_id == NULL)
    {
        //If client hasn't logged in yet
        msg2.messageID = ERROR_FAILED_AUTHENTICATION;
    }
    else
    {
        //Finds the group table
        group = table_get(&groups_table, client->group_id);

        //Check if the group table exists
        if(group != NULL)
        {
            //If the group table exists, then try to get the value
            value = table_get(group, msg->firstArg);
        }

        // If value == NULL, not found, else it is found and sent to client
        if (value == NULL)
        {
            msg2.messageID = ERROR_VALUE_NOT_FOUND;
        }
        else
        {
            msg2.messageID = MSG_OKAY;
            msg2.secondArg = value;
        }
    }

    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}

int msg_received_delete(Client *client, Message *msg)
{
    HashTable *group = NULL;
    Message msg2 = { .messageID = -1, .firstArg = NULL, .secondArg = NULL };

    if(client->group_id == NULL)
    {
        //If client hasn't logged in yet
        msg2.messageID = ERROR_FAILED_AUTHENTICATION;
    }
    else
    {
        //Finds the group table
        group = table_get(&groups_table, client->group_id);
        // If value == NULL, not found, else it is found and sent to client
        if (group != NULL && table_delete(group, msg->firstArg) == 0) //successfully deleted
        {
            value_for_key_modified(msg->firstArg);
            msg2.messageID = MSG_OKAY;
            msg2.firstArg = NULL;
            msg2.secondArg = NULL;
        }
        else
        {
            msg2.messageID = ERROR_VALUE_NOT_FOUND; 
            msg2.firstArg = NULL;
            msg2.secondArg = NULL;
        }
    }

    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return (1); //success
}


int msg_received_register_callback(Client *client, Message *msg)
{
    char* key = msg->firstArg;
    printf("Client %p wants to be warned when key %s is changed\n", (void*)client, key);

    //New client list item
    Client_List_Item* item = (Client_List_Item*)malloc(sizeof(Client_List_Item));
    item->client = client;
    item->next = NULL;

    /* Add the client to a list on the hashtable value for that key  */

    //First, get the existing list of clients w/ callback for that key
    Client_List_Item* list = (Client_List_Item*)table_get(&clients_with_callback_by_key, key);
    if(list == NULL)
    {
        //Create a new list with that client as the only item
        table_insert(&clients_with_callback_by_key, key, (void*)item);
    }
    else
    {
        //List already exists, append this client to that list
        while(list->next != NULL)
            list = list->next;

        list->next = item;
    }    

    //Send to the client it was successfuly registered
    Message msg2 = { .messageID = MSG_OKAY, .firstArg = NULL, .secondArg = NULL };
    if (send_message(client->sockFD, msg2) == -1)
    {
        return (-1);
    }

    return 1;
}

/**
 * @brief  Send callback messages to any client that is interested in it
 * @note   
 * @param  key: 
 * @retval None
 */
void value_for_key_modified(char* key)
{
    printf("TODO: notify clients this key has changed");
    //clients_with_callback_by_key
}

/**
 * @brief  Generates a random session_id for a client. This is used to identify when it want's to establish a connection
 *          with the callback socket
 * @note   
 * @param  *str: a prealocated string of size SESSION_ID_STR_SIZE where the value is written to
 * @retval None
 */
void generate_random_session_id(char *str)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789!#$&/()=}][{";

    for (size_t n = 0; n < SESSION_ID_STR_SIZE-1; n++) {
        int key = rand() % (int) (sizeof(charset) - 1);
        str[n] = charset[key];
    }
    str[SESSION_ID_STR_SIZE-1] = '\0';
}