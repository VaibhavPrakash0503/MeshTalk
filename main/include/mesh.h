#pragma once

#include "esp_ble_mesh_defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Callback type for raw incoming messages
 */
typedef void (*mesh_receive_cb_t)(const uint8_t *data, size_t len);

/**
 * @brief Send a raw buffer through BLE Mesh vendor model
 */
int mesh_send_raw(const uint8_t *data, size_t data_len, uint16_t receiver_add);

/**
 * @brief Register a callback for raw incoming data
 */
void mesh_register_receive_cb(mesh_receive_cb_t cb);

bool mesh_broadcast_self(const uint8_t *buffer, size_t buffer_len);

void mesh_vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                          esp_ble_mesh_model_cb_param_t *param);
