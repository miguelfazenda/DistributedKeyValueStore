#include "ui.h"
#include <stdbool.h>
#include <string.h>

#include "globals.h"
#include "../shared/auth_defines.h"
#include "auth.h"

void read_terminal(char *word);

void run_ui(void)
{
    //Terminal reading
    printf("Welcome! Here are the commands for your operations:\n____________________\n");
    printf("create group: 'create'\ndelete group:'delete'\nshow group info:'group'\nquit\n");
    printf("show application status: 'status'\n");
    printf("____________________\n\n");

    bool quitting = false;
    while(!quitting)
    {
        // Read operation from terminal
        char operation[100];
        printf("Enter operation: ");
        read_terminal(operation);

        if (strcmp(operation, "create") == 0)
        {
            char group_id[1000];
            char group_secret[SECRET_SIZE];

            // Read group ID
            printf("Insert group ID you want to create: ");
            read_terminal(group_id);

            // Send command to auth server, and received a generated random secret
            int8_t status = auth_create_group(group_id, group_secret);
            if (status == 1)
            {
                printf("Group created successfully!\nGroup: %s\nSecret: %s\n", group_id, group_secret);
            }
            else
            {
                printf("Error creating group: %s\n", get_error_code_string(status, "error"));
            }
        }
        else if (strcmp(operation, "delete") == 0)
        {
            char group_id[1000];

            // Read group ID
            printf("Insert group ID to delete: ");
            read_terminal(group_id);
            
            //Delete table from groups table
            pthread_mutex_lock(&groups_table_mtx);
            table_delete(&groups_table, group_id);
            pthread_mutex_unlock(&groups_table_mtx);

            //Delete group in auth server
            auth_delete_group(group_id);
        }
        else if (strcmp(operation, "group") == 0)
        {
            char group_id[1000];
            char group_secret[SECRET_SIZE];

            // Read group ID
            printf("Insert group ID to know information: ");
            read_terminal(group_id);

            int8_t status = auth_get_secret(group_id, group_secret);
            if (status == 1)
            {
                pthread_mutex_lock(&groups_table_mtx);
                HashTable* table_of_group = (HashTable*)table_get(&groups_table, group_id);
                int num_pairs = table_of_group == NULL ? 0 : table_count_pairs(table_of_group);
                pthread_mutex_unlock(&groups_table_mtx);

                printf("Secret: %s\nNumbers of pairs key/value:%d\n", group_secret, num_pairs);
            }
            else
            {
                printf("Error getting group info: %s\n", get_error_code_string(status, "error"));
            }
        }
        else if (strcmp(operation, "status") == 0)
        {
            //STATUS
            pthread_mutex_lock(&clients.mtx_client_list);

            int i = 0;
            Client* client = clients.client_list;

            if(client == NULL)
            {
                printf("No client has yet connected\n");
            }

            while(client != NULL)
            {
                struct tm * time_info;
                char connected_time[9]; //"HH:MM:SS\0"
                char disconnected_time[9]; //"HH:MM:SS\0"

                time_info = localtime(&client->time_connected);
                strftime(connected_time, sizeof(connected_time), "%H:%M:%S", time_info);
                

                double connection_time;
                if(client->connected)
                {
                    connection_time = difftime(time(NULL), client->time_connected);
                }
                else
                {
                    //Already disconnected
                    connection_time = difftime(client->time_disconnected, client->time_connected);

                    time_info = localtime(&client->time_disconnected);
                    strftime(disconnected_time, sizeof(disconnected_time), "%H:%M:%S", time_info);
                }

                printf("Client %d:\n\t%s\n\tPID %d\n\tConnection time %g seconds\n\tConnected at %s\n",
                    i,
                    (client->connected ? "Connected" : "Disconnected"),
                    client->pid,
                    connection_time,
                    connected_time);

                if(!client->connected)
                    printf("\tDisconnected at %s\n", disconnected_time);

                i++;
                client = client->next;
            }

            pthread_mutex_unlock(&clients.mtx_client_list);
        }
        else if (strcmp(operation, "quit") == 0)
        {
            //Exits the loop
            quitting = true;
        }
    }
}

void read_terminal(char *word)
{
    while (fscanf(stdin, "%s", word) != 1)
    {
        printf("Invalid input. Insert again");
    }
}