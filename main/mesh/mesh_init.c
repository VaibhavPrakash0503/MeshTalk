#include "mesh_init.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_log.h"
#include "vendor_model.h"

static const char *TAG = "MESH_INIT";

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/* -------------------------------------------------------------------------- */
/*                          Static Keys & UUIDs                               */
/* -------------------------------------------------------------------------- */

/* Device UUID (unique per device) */
uint8_t dev_uuid[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

/* Static NetKey and AppKey (for static provisioning) */
const uint8_t net_key[16] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
                             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};

const uint8_t app_key[16] = {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89,
                             0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89};

/* Static OOB (for provisioning authentication) */
static uint8_t static_oob_val[16] = {[0 ... 15] = 0xAA};

/* Node unicast address (defined in node_config.h) */
uint16_t unicast_addr = UNICAST_ADDR;

/* -------------------------------------------------------------------------- */
/*                       Provisioning & Composition                           */
/* -------------------------------------------------------------------------- */

/* Provisioning parameters */
static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
    .static_val = static_oob_val,
    .static_val_len = sizeof(static_oob_val),
};

/* Vendor model callback & opcode table are defined in vendor_model.c */
extern esp_ble_mesh_model_op_t op_vendor[];
extern void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                            esp_ble_mesh_model_cb_param_t *param);

/* AppKey index for binding */
static uint16_t vendor_app_idx = APP_IDX;

/* Vendor model instance */
esp_ble_mesh_model_t vendor_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, VENDOR_MODEL_ID, op_vendor,
                              vendor_model_cb, NULL, &vendor_app_idx),
};

/* Elements */
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, ARRAY_SIZE(vendor_models), vendor_models),
};

/* Composition */
static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

/* -------------------------------------------------------------------------- */
/*                          Init Function                                     */
/* -------------------------------------------------------------------------- */

void mesh_init(void) {
  esp_err_t err;

  ESP_LOGI(TAG, "Initializing BLE Mesh Node (Static Provisioning)");

  /* Register provisioning callback */
  err = esp_ble_mesh_register_prov_callback(provisioning_cb);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register provisioning callback (err 0x%X)", err);
    return;
  }

  /* Initialize BLE Mesh stack */
  err = esp_ble_mesh_init(&prov, &composition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize mesh stack (err 0x%X)", err);
    return;
  }

  /* Add static NetKey */
  err = esp_ble_mesh_node_add_local_net_key(net_key, NET_IDX);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add NetKey (err 0x%X)", err);
  } else {
    ESP_LOGI(TAG, "NetKey added successfully");
  }

  /* Add static AppKey */
  err = esp_ble_mesh_node_add_local_app_key(app_key, NET_IDX, APP_IDX);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add AppKey (err 0x%X)", err);
  } else {
    ESP_LOGI(TAG, "AppKey added successfully");
  }

  /* Bind AppKey to vendor model */
  err = esp_ble_mesh_node_bind_app_key_to_local_model(
      APP_IDX, VENDOR_MODEL_ID, elements[0].element_addr, CID_ESP);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to bind AppKey to vendor model (err 0x%X)", err);
  } else {
    ESP_LOGI(TAG, "AppKey bound to vendor model");
  }

  /* Enable node provisioning (advertising + GATT) */
  err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV |
                                      ESP_BLE_MESH_PROV_GATT);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable provisioning (err 0x%X)", err);
    return;
  }

  ESP_LOGI(TAG, "BLE Mesh Node initialized and ready");
  ESP_LOGI(TAG, "Device UUID: %02X%02X%02X%02X...", dev_uuid[0], dev_uuid[1],
           dev_uuid[2], dev_uuid[3]);
}
