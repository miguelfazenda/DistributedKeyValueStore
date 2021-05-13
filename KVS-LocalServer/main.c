#include "hashtable.h"

int main()
{
    HashTable table = table_create();
    table_insert(&table, "miguel", "fixe");
    table_insert(&table, "ab", "ola1");
    table_insert(&table, "ba", "ola2");

    printf("miguel -> %s", table_get(&table, "miguel"));
    printf("ab -> %s", table_get(&table, "ab"));
    printf("ba -> %s", table_get(&table, "ba"));

    table_delete(&table, "ab");

    //DEvia dar um apontador para NULL, pq removemos o ab
    printf("ab -> pointer: %s", table_get(&table, "ab"));
}