#ifndef HASHTABLE_H
#define HASHTABLE_H

#define TABLE_SIZE 20

typedef struct HashTable_struct
{
    struct TableItem* array[TABLE_SIZE];
} HashTable;

typedef struct TableItem_struct
{
    char* key;
    void* value;
    struct TableItem* next;
} TableItem;

HashTable table_create();
void table_insert(HashTable* table, char* key, void* value);
void* table_get(HashTable* table, char* key);
void table_delete(HashTable* table, char* key);

#endif