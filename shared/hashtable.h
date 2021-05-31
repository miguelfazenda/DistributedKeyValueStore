#ifndef HASHTABLE_H
#define HASHTABLE_H

#define TABLE_SIZE 20

typedef struct TableItem_struct
{
    char* key;
    void* value;
    struct TableItem_struct* next;
} TableItem;

typedef struct HashTable_struct
{
    TableItem* array[TABLE_SIZE];

    //The function used to free the values, when table_delete is called
    void (*free_value_func)(void*);
} HashTable;

HashTable table_create(void (*free_value_func)(void*));
void table_insert(HashTable* table, const char* key, void* value);
void* table_get(HashTable* table, const char* key);
int table_delete(HashTable* table, const char* key);
void table_free(HashTable* table);

void free_value_str(void*);
void free_value_hashtable(void*);

#endif
