#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Callback type for raw incoming messages
 */
typedef void (*mesh_receive_cb_t)(const uint8_t *data, size_t len);

/**
 * @brief Send a raw buffer through BLE Mesh vendor model
 */
int mesh_send_raw(const uint8_t *data, size_t data_len);

/**
 * @brief Register a callback for raw incoming data
 */
void mesh_register_receive_cb(mesh_receive_cb_t cb);
