#ifndef __MESSAGE_HANDLING_H
#define __MESSAGE_HANDLING_H

#include "client_list.h"
#include "../shared/message.h"

int msg_received_put(Client* client, Message* msg);
int msg_received_login(Client* client, Message* msg);

/*#define MAX_HANDLING_FUNCTION_ID 9
int (*message_handling_functions[MAX_HANDLING_FUNCTION_ID + 1])(Client*, Message*);*/

#endif