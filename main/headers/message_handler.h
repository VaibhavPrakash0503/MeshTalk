#pragma once

#include "message_struct.h"
#include <stddef.h>
#include <stdint.h>

/**
 * Application-level callback (receives fully processed MeshMessage)
 */
typedef void (*app_receive_cb_t)(const MeshMessage *msg);

/**
 * Initialize the message handler and hook into mesh
 */
void message_handler_init(void);

/**
 * Send a fully processed MeshMessage
 */
int message_handler_send(const MeshMessage *msg, uint16_t receiver_add);

/**
 * Internal: receive raw data from mesh (not for app use)
 */
void message_handler_receive_raw(const uint8_t *data, size_t len);

/**
 * Process an incoming raw buffer (verify CRC, deserialize)
 */
int message_handler_process_incoming(const uint8_t *data, size_t len,
                                     MeshMessage *out_msg);

/**
 * Register app callback for processed messages
 */
void message_handler_register_app_cb(app_receive_cb_t cb);
