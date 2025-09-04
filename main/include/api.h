#pragma once

#include "message_struct.h"
#include <stdbool.h>
#include <stdint.h>

// Callback type for UI to receive messages
typedef void (*ui_receive_cb_t)(const char *sender_name, const char *msg);

// Initialize API and register UI callback
void api_init(ui_receive_cb_t cb);

// UI → API → Handler
void api_send_text(const char *receiver_id, const char *msg);

void api_on_message_received(const MeshMessage *m);

void api_broadcast_addr(void);

int api_get_contact_names(char names[][USERNAME_MAX_LEN], int max_contacts);

// Get chat log of a user by name
// out_buf: 2D array to store messages
// max_msgs: maximum messages to copy
// Returns the number of messages copied
int api_get_chat_log(const char *username, char out_buf[][MAX_MESSAGE_LEN + 1],
                     int max_msgs);
