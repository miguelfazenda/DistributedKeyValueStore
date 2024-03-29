#ifndef __ERROR_CODES_H
#define __ERROR_CODES_H

#include <stdint.h>

#define ERROR_WRONG_SECRET -2
#define ERROR_FAILED_AUTHENTICATION -3
#define ERROR_AUTH_GROUP_NOT_PRESENT -4
#define ERROR_DISCONNECTION_WARNING_FAILED -5 //Failed warning other end that it is disconnecting
#define ERROR_SENDING -6
#define ERROR_RECEIVING -7
#define ERROR_VALUE_NOT_FOUND -8
#define ERROR_FAILED_CONNECTING -9
#define ERROR_RECV_TIMEOUT -10 //recv function timed out
#define ERROR_AUTH_GROUP_ALREADY_EXISTS -11
#define ERROR_AUTH_SERVER_UNREACHABLE -12

const char *get_error_code_string(int8_t code, const char *generic_error);

#endif