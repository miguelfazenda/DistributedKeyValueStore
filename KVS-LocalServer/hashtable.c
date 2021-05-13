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
int table_hash_function(char* key)
{
    int sum = 0;
    for(size_t i = 0; i < strlen(key); i++)
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
HashTable table_create()
{
    HashTable table;
    
    for(size_t i = 0; i < TABLE_SIZE; i++)
    {
        table.array[i] = NULL;
    }
    
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
void table_insert(HashTable* table, char* key, void* value)
{
    int index = table_hash_function(key);

    //Finds the pointer to the pointer where the first item is stored
    TableItem** itemPos = &table->array[index];
    
    //If itemPos doen't point to NULL, advance in the list
    while(*itemPos != NULL)
    {
        itemPos = &(*itemPos)->next;
    }

    *itemPos = malloc(sizeof(TableItem));

    (*itemPos)->key = key;
    (*itemPos)->value = value;
    (*itemPos)->next = NULL;
}

void* table_get(HashTable* table, char* key)
{
    int index = table_hash_function(key);

    TableItem* itemPos = table->array[index];
    
    while(itemPos != NULL)
    {
        if(strcmp(itemPos->key, key) == 0)
        {
            return itemPos->value;
        }
        
        itemPos = itemPos->next;
    }

    //If no value for this key was found, return NULL
    return NULL;
}

void table_delete(HashTable* table, char* key)
{

}
