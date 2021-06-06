#include "error_codes.h"

/**
 * @brief  Translates an error code into a string 
 * @param  code: The error code
 * @param  generic_error: If there is no string for such error code, returns this string
 */
const char *get_error_code_string(int8_t code, const char *generic_error)
{
    if (code == ERROR_WRONG_SECRET)
        return "Wrong secret!";
    else if (code == ERROR_AUTH_GROUP_NOT_PRESENT)
        return "Group not present in the server!";
    else if (code == ERROR_FAILED_AUTHENTICATION)
        return "Failed authentication.";
    else if (code == ERROR_SENDING)
        return "Failed sending information to server.";
    else if (code == ERROR_RECEIVING)
        return "Failed receiving information from server.";
    else if (code == ERROR_VALUE_NOT_FOUND)
        return "Value not found.";
    else if (code == ERROR_AUTH_GROUP_ALREADY_EXISTS)
        return "Group already exists.";

    return generic_error;
}