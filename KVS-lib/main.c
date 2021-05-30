#include "KVS-lib.h"
#include "../shared/message.h"
#include "../shared/error_codes.h"
#include <stdio.h>

//TODO Tirar isto daqui
/**
 * @brief  Translates an error code into a string 
 * @param  code: The error code
 * @param  generic_error: If there is no string for such error code, returns this string
 */
const char* get_error_code_string(int8_t code, const char* generic_error)
{
    if(code == ERROR_WRONG_SECRET)
        return "Wrong secret!";
    else if(code == ERROR_AUTH_GROUP_NOT_PRESENT)
        return "Group not present in the server!";

    return generic_error;
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
        const char* error_str = get_error_code_string(status, "Error establishing connection");
        printf("%s\n", error_str);
    }

    status = close_connection();
    if(status == 1)
        printf("Disconnected successfuly\n");
    else
        printf("%s\n", get_error_code_string(status, "Error disconnecting"));
}