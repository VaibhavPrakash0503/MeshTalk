#pragma once

#include "esp_ble_mesh_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Static NetKey index */
#define NET_IDX 0x0000

/** @brief Static AppKey index */
#define APP_IDX 0x0000

/** @brief Company ID (Espressif example) */
#define CID_ESP 0x02E5

/** @brief Vendor Model ID */
#define VENDOR_MODEL_ID 0x0000

/** @brief Unicast address for this node */
#define UNICAST_ADDR 0x0001

/** @brief Hardcoded NetKey (16 bytes) */
extern const uint8_t net_key[16];

/** @brief Hardcoded AppKey (16 bytes) */
extern const uint8_t app_key[16];

/** @brief Hardcoded Device UUID (16 bytes) */
extern uint8_t dev_uuid[16];

/**
 * @brief Initialize BLE Mesh stack with static provisioning.
 *
 * This function sets up:
 *  - Provisioning information
 *  - Composition data
 *  - Vendor model
 *  - Static NetKey & AppKey
 *  - Starts the mesh node
 */
void mesh_init(void);

#ifdef __cplusplus
}
#endif
