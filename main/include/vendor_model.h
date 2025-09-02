#pragma once

#include "esp_ble_mesh_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Vendor Company ID (Espressif example) */
#define VENDOR_COMPANY_ID 0x02E5

/** Vendor Model ID (must match mesh_init.h) */
#define VENDOR_MODEL_ID 0x0000

/** Vendor Opcodes */
#define VENDOR_OPCODE_MESSAGE ESP_BLE_MESH_MODEL_OP_3(0x01, VENDOR_COMPANY_ID)
#define VENDOR_OPCODE_ACK ESP_BLE_MESH_MODEL_OP_3(0x02, VENDOR_COMPANY_ID)

/** Opcode table (exposed for mesh_init.c) */
extern esp_ble_mesh_model_op_t op_vendor[];

/**
 * @brief Vendor Model callback (registered in mesh_init.c)
 */
void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                     esp_ble_mesh_model_cb_param_t *param);

/**
 * @brief Provisioning callback (registered in mesh_init.c)
 */
void provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                     esp_ble_mesh_prov_cb_param_t *param);

#ifdef __cplusplus
}
#endif
