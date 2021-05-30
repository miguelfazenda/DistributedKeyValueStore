#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "hashtable.h"

/**
 * @brief Generates a hash of a string. It's the sum of all characters
 * 
 * @param key The String
 * @return int The hash value between 0 and TABLE_SIZE-1
 */
int table_hash_function(char *key)
{
    int sum = 0;
    for (size_t i = 0; i < strlen(key); i++)
    {
        sum += (int)key[i];
    }

    return sum % TABLE_SIZE;
}

/**
 * @brief Creates a hash table. Initializing the vector with nulls
 * 
 * @return HashTable The table
 */
HashTable table_create(void (*free_value_func)(void *))
{
    HashTable table;

    for (size_t i = 0; i < TABLE_SIZE; i++)
    {
        table.array[i] = NULL;
    }

    table.free_value_func = free_value_func;

    return table;
}

//TODO: Isto nao precisa de enviar o table*, só table
/**
 * @brief Inserts a value on the hashtable
 * 
 * @param table 
 * @param key 
 * @param value 
 */
// Inserts a value for a specific key
void table_insert(HashTable *table, char *key, void *value)
{
    int index = table_hash_function(key);

    //Finds the pointer to the pointer where the first item is stored
    TableItem **itemPos = &table->array[index];

    while (*itemPos != NULL)
    {
        if (strcmp((*itemPos)->key, key) == 0)
        {
            table->free_value_func((*itemPos)->value);
            (*itemPos)->value = value;
            break;
        }

        *itemPos = (*itemPos)->next;
    }

    if (*itemPos == NULL)
    {
        *itemPos = malloc(sizeof(TableItem));

        //Allocate and copy the key, because the original key string may be free-ed later
        (*itemPos)->key = (char *)malloc((strlen(key) + 1) * sizeof(char));
        strcpy((*itemPos)->key, key);

        (*itemPos)->value = value;
        (*itemPos)->next = NULL;
    }
}

// Finds the value/group for a specific key/id. If found, return pointer for value. If not found, return NULL
void *table_get(HashTable *table, char *key)
{
    int index = table_hash_function(key);

    TableItem *itemPos = table->array[index];

    while (itemPos != NULL)
    {
        if (strcmp(itemPos->key, key) == 0)
        {
            return itemPos->value;
        }

        itemPos = itemPos->next;
    }

    //If no value for this key was found, return NULL
    return NULL;
}

// Deletes table. If it exists, is deleted (return 0). If it doesnt exist, return(-1)
int table_delete(HashTable *table, char *key)
{
    int index = table_hash_function(key);

    //Finds the pointer to the pointer where the first item is stored
    TableItem **itemPos = &table->array[index];

    //If itemPos doen't point to NULL, advance in the list
    while (*itemPos != NULL)
    {
        if (strcmp((*itemPos)->key, key))
        {
            //Found the correct list entry, delete it
            free((*itemPos)->key);
            table->free_value_func((*itemPos)->value);
            free(*itemPos);

            *itemPos = (*itemPos)->next;
            return(0); //successfully deleted
         }

        itemPos = &(*itemPos)->next;
    }

    return(-1); //does not exist
}

/**
 * @brief Frees the table (doesnt free the table*)
 * 
 * @param table 
 */
void table_free(HashTable *table)
{
    printf("UNIMPLEMENTED! THIS SHOULD FREE ALL OF THE TABLE'S CONTENTS\n");
}

void free_value_str(void *ptr)
{
    free(ptr);
}

void free_value_hashtable(void *ptr)
{
    table_free((HashTable *)ptr);
}