#pragma once

#include "message_handler.h"
#include <stdint.h>

// Callback type for UI to receive messages
typedef void (*ui_receive_cb_t)(const char *sender_name, const char *msg);

// Initialize API and register UI callback
void api_init(ui_receive_cb_t cb);

// UI → API → Handler
void api_send_text(const char *receiver_id, const char *msg);

void api_on_message_received(uint16_t sender_addr, const MeshMessage *m);
