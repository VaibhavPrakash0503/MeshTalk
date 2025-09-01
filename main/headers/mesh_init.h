#pragma once

#include "esp_ble_mesh_defs.h"
#include "node_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Static NetKey index */
#define NET_IDX 0x0000

/** Static AppKey index */
#define APP_IDX 0x0000

/** Company ID (Espressif example) */
#define CID_ESP 0x02E5

/** Vendor Model ID */
#define VENDOR_MODEL_ID 0x0000

/** Unicast address for this node
 * defined in the node_config.h */

/** Hardcoded NetKey (16 bytes) */
extern const uint8_t net_key[16];

/** Hardcoded AppKey (16 bytes) */
extern const uint8_t app_key[16];

/** Hardcoded Device UUID (16 bytes) */
extern uint8_t dev_uuid[16];

/** Vendor model array (exported for mesh.c or model_vendor.c use) */
extern esp_ble_mesh_model_t vendor_models[];

/** Net/App indices & address */
extern uint16_t unicast_addr;

/**
 * @brief Initialize BLE Mesh stack with static provisioning.
 */
void mesh_init(void);

#ifdef __cplusplus
}
#endif
