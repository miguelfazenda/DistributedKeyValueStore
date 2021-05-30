#include "KVS-lib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char operation[10];
    char first_arg[10];
    char second_arg[10];
    char *value;
    char connected = 0;

    printf("Welcome! Here are the commands for your operations:\n____________________\nauthentication: 'login'\ninsert a new value:'insert'\n____________________\n\n");
    while (1)
    {
        printf("Opperation wanted: ");
        while (fscanf(stdin, "%s", operation) != 1)
        {
            printf("Invalid input. Insert again");
        }

        if ((strcmp(operation, "login") == 0) && connected==0 )
        {
            printf("Enter group ID: ");
            while (fscanf(stdin, "%s", first_arg) != 1)
            {
                printf("Invalid input. Insert again");
            }

            printf("Enter secret: ");
            while (fscanf(stdin, "%s", second_arg) != 1)
            {
                printf("Invalid input. Insert again");
            }

            if (establish_connection(first_arg, second_arg) == 0)
            {
                printf("Connection established.\n");
                connected = 1;
            }
        }
        else if ((strcmp(operation, "login") == 0) && connected==1)
        {
            printf("You're already logged in. Insert a valid operation.\n");
        }
        else if (connected == 1)
        {
            if (strcmp(operation, "insert") == 0)
            {
                printf("Enter key: ");
                while (fscanf(stdin, "%s", first_arg) != 1)
                {
                    printf("Invalid input. Insert again");
                }

                printf("Enter value: ");
                while (fscanf(stdin, "%s", second_arg) != 1)
                {
                    printf("Invalid input. Insert again");
                }

                if ((put_value(first_arg, second_arg)) == 1)
                {
                    printf("Successful insert!\n");
                }
                else
                {
                    printf("Error insert!\n");
                }
            }
            else if (strcmp(operation, "get") == 0)
            {
                printf("Enter key: ");
                while (fscanf(stdin, "%s", first_arg) != 1)
                {
                    printf("Invalid input. Insert again");
                }

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
}