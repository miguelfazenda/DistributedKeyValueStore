#include "KVS-lib.h"
#include "../shared/message.h"
#include <stdio.h>

int main(void)
{
    int status = establish_connection((const char*)"groupId", (const char*)"Secret1");
    if(status == 0)
    {
        printf("Established connection successfully\n");
    }
    else if(status == ERROR_WRONG_SECRET)
    {
        printf("Wrong secret!\n");
    }
    else
    {
        printf("Error establishing connection\n");
    }
}