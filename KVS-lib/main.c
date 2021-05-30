#include "KVS-lib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../shared/message.h"
#include "../shared/error_codes.h"

void read_terminal(char* word);
int login_m();
void get_m();
void delete_m();
void insert_m();

//TODO Tirar isto daqui
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

    return generic_error;
}

int main(void)
{
    char operation[10];
    char connected = 0;

    printf("Welcome! Here are the commands for your operations:\n____________________\nauthentication: 'login'\ninsert a new value:'insert'\n____________________\n\n");
    while (1)
    {
        printf("Opperation wanted: ");
        read_terminal(operation);

        if ((strcmp(operation, "login") == 0) && connected == 0)
        {
            connected = login_m();
        }
        else if ((strcmp(operation, "login") == 0) && connected == 1)
        {
            printf("You're already logged in. Insert a valid operation.\n");
        }
        else if (connected == 1)
        {
            if (strcmp(operation, "insert") == 0)
            {
                insert_m();
            }
            else if (strcmp(operation, "get") == 0)
            {
                get_m();
            }
            else if (strcmp(operation, "delete") == 0)
            {
                delete_m();
            }
            else
            {
                printf("Insert a valid operation\n");
            }
        }
        else
        {
            printf("You're not connected. Please log in first.\n");
        }

        printf("\n-----------------\n");
    }

    close_connection();
}

void read_terminal(char *word)
{
    while (fscanf(stdin, "%s", word) != 1)
    {
        printf("Invalid input. Insert again");
    }
}

int login_m()
{
    char first_arg[100];
    char second_arg[100];

    printf("Enter group ID: ");
    read_terminal(first_arg);

    printf("%s\n", first_arg);

    printf("Enter secret: ");
    read_terminal(second_arg);

    if (establish_connection(first_arg, second_arg) == 0)
    {
        printf("Connection established.\n");
        return (1);
    }
    return (0);
}

void insert_m()
{
    char first_arg[100];
    char second_arg[100];

    printf("Enter key: ");
    read_terminal(first_arg);

    printf("Enter value: ");
    read_terminal(second_arg);

    if ((put_value(first_arg, second_arg)) == 1)
    {
        printf("Successful insert!\n");
    }
    else
    {
        printf("Error insert!\n");
    }
}

void get_m()
{
    char first_arg[100];
    char *value;

    printf("Enter key: ");
    read_terminal(first_arg);

    if ((get_value(first_arg, &value)) == 1)
    {
        printf("Key inserted: %s. Value delivered: %s\n", first_arg, value);
        free(value);
    }
    else
    {
        printf("Error finding the value!\n");
    }
}

void delete_m()
{
    char first_arg[100];

    printf("Enter key: ");
    read_terminal(first_arg);

    if ((delete_value(first_arg)) == 1)
    {
        printf("Successfully deleted\n");
    }
    else
    {
        printf("Error deleting the value!\n");
    }
}

