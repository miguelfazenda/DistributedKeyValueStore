#ifndef __MESSAGE_HANDLING_H
#define __MESSAGE_HANDLING_H

#include "client_list.h"
#include "../shared/message.h"

int msg_received_put(Client* client, Message* msg);
int msg_received_login(Client* client, Message* msg);
int msg_received_get(Client *client, Message *msg);
int msg_received_delete(Client *client, Message *msg);
int msg_received_register_callback(Client *client, Message *msg);

#endif