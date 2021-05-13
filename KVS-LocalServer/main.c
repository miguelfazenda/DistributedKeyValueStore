#include "hashtable.h"
#include <stdio.h>

int main()
{
    HashTable table = table_create();
    table_insert(&table, "miguel", "fixe");
    table_insert(&table, "ab", "ola1");
    table_insert(&table, "ba", "ola2");

    printf("miguel -> %s\n", (char *)table_get(&table, "miguel"));
    printf("ab -> %s\n", (char *)table_get(&table, "ab"));
    printf("ba -> %s\n", (char *)table_get(&table, "ba"));

    table_delete(&table, (void *)"ab");

    //DEvia dar um apontador para NULL, pq removemos o ab
    printf("ab -> pointer: %p\n", table_get(&table, "ab"));
    printf("ab -> pointer: %p\n", table_get(&table, "bbbb"));
}
