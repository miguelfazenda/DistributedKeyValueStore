#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "hashtable.h"

int table_hash_function(const char* key);

/**
 * @brief Generates a hash of a string. It's the sum of all characters
 * 
 * @param key The String
 * @return int The hash value between 0 and TABLE_SIZE-1
 */
int table_hash_function(const char* key)
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
 * @param free_value_func: The function that will be run when we delete/free values
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
void table_insert(HashTable* table, const char* key, void* value)
{
    int index = table_hash_function(key);

    //Finds the pointer to the pointer where the first item is stored
    TableItem **itemPos = &table->array[index];

    while (*itemPos != NULL)
    {
        if (strcmp((*itemPos)->key, key) == 0)
        {
            if(table->free_value_func != NULL)
                table->free_value_func((*itemPos)->value);
            (*itemPos)->value = value;
            break;
        }

        itemPos = &(*itemPos)->next;
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
void *table_get(HashTable *table, const char *key)
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
int table_delete(HashTable *table, const char *key)
{
    //TODO avisar se não encontrar?
    int index = table_hash_function(key);

    //Finds the pointer to the pointer where the first item is stored
    TableItem **itemPos = &table->array[index];

    //If itemPos doen't point to NULL, advance in the list
    while (*itemPos != NULL)
    {
        if (strcmp((*itemPos)->key, key) == 0)
        {
            //Found the correct list entry
            //Store the next
            TableItem* next = (*itemPos)->next;

            //delete it
            free((*itemPos)->key);
            if(table->free_value_func != NULL)
                table->free_value_func((*itemPos)->value);
            free(*itemPos);

            //Set the current itemPos to the next
            *itemPos = next;
            return(0); //successfully deleted
         }

        itemPos = &(*itemPos)->next;
    }

    return(-1); //does not exist
}

/**
 * @brief Frees the table (doesnt free the table*). Frees both the key and value
 * 
 * @param table 
 */
void table_free(HashTable *table)
{
    for(int i = 0; i<TABLE_SIZE; i++)
    {
        TableItem *item = table->array[i];

        while (item != NULL)
        {
            TableItem* next = item->next;
            
            //Frees the item
            free(item->key);
            if(table->free_value_func != NULL)
                table->free_value_func(item->value);
            free(item);

            item = next;
        }
    }
}

int table_count_pairs(HashTable* table)
{
    int count = 0;
    TableItem* item;

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        item = table->array[i];
        while(item != NULL)
        {
            count++;
            item = (item)->next;
        }
    }

    return(count);
}

void free_value_str(void *ptr)
{
    free(ptr);
}

void free_value_hashtable(void *ptr)
{
    table_free((HashTable *)ptr);
    free(ptr);
}