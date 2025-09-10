#pragma once

#include "esp_ble_mesh_defs.h"
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

/** Hardcoded NetKey (16 bytes) */
extern const uint8_t net_key[16];

/** Hardcoded AppKey (16 bytes) */
extern const uint8_t app_key[16];

/** Hardcoded Device UUID (16 bytes) */
extern uint8_t dev_uuid[16];

/** Vendor model array (exported for mesh.c or model_vendor.c use) */
extern esp_ble_mesh_model_t vendor_models[];

/**
 * @brief Initialize BLE Mesh stack with static provisioning.
 */
void mesh_init(void);

/**
 * @brief Provisioning callback (implemented in mesh_init.c).
 */
void provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                     esp_ble_mesh_prov_cb_param_t *param);

void check_provisioning_status(void);

uint16_t get_net_idx(void);
uint16_t get_app_idx(void);
uint16_t get_node_addr(void);
bool is_provisioned(void);

#ifdef __cplusplus
}
#endif
