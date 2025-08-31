#pragma once

#include "message_handler.h"
#include <stdint.h>

// Callback type for UI to receive messages
typedef void (*ui_receive_cb_t)(uint16_t sender_id, const char *msg);

// Initialize API and register UI callback
void api_init(ui_receive_cb_t cb);

// UI → API → Handler
void api_send_text(uint16_t receiver_id, const char *msg);
