#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "../shared/hashtable.h"


int main(void)
{
    HashTable table_of_tables = table_create(free_value_hashtable);

    HashTable table = table_create(free_value_str);

    table_insert(&table_of_tables, "table1", &table);

    table_insert(&table, "miguel", strdup("fixe"));
    table_insert(&table, "ab", strdup("ola1"));
    table_insert(&table, "ba", strdup("ola2"));
    table_insert(&table, "ab", strdup("ola3"));

    printf("miguel -> %s\n", (char *)table_get(&table, "miguel"));
    printf("ab -> %s\n", (char *)table_get(&table, "ab"));
    printf("ba -> %s\n", (char *)table_get(&table, "ba"));

    table_delete(&table, "ab");

    //DEvia dar um apontador para NULL, pq removemos o ab
    printf("ab -> pointer: %p\n", table_get(&table, "ab"));
    printf("ab -> pointer: %p\n", table_get(&table, "bbbb"));

    table_free(&table_of_tables);

    return 0;
}