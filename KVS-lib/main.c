#include "KVS-lib.h"
#include "../shared/message.h"
#include "../shared/error_codes.h"
#include <stdio.h>

//TODO Tirar isto daqui
const char* get_error_code_string(int8_t code)
{
    if(code == ERROR_WRONG_SECRET)
        return "Wrong secret!";
    else if(code == ERROR_AUTH_GROUP_NOT_PRESENT)
        return "Group not present in the server!";

    return NULL;
}

int main(void)
{
    int status = establish_connection((const char*)"groupId", (const char*)"Secret1");
    if(status == 0)
    {
        printf("Established connection successfully\n");
    }
    else
    {
        //Print the error related to the error code stored in status
        const char* error_str = get_error_code_string(status);
        if(error_str != NULL)
            printf("%s\n", error_str);
        else
            printf("Error establishing connection\n");
    }
}