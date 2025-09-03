#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Callback type for raw incoming messages
 */
typedef void (*mesh_receive_cb_t)(uint16_t sender_addr, const uint8_t *data,
                                  size_t len);

/**
 * @brief Send a raw buffer through BLE Mesh vendor model
 */
int mesh_send_raw(const uint8_t *data, size_t data_len, uint16_t receiver_add);

/**
 * @brief Register a callback for raw incoming data
 */
void mesh_register_receive_cb(mesh_receive_cb_t cb);

bool mesh_broadcast_self(void);
