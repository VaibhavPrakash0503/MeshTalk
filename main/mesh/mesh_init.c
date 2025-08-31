#include "mesh_init.h"
#include "model_vendor.h"

#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_log.h"

static const char *TAG = "MESH_INIT";

/** Hardcoded keys and UUID */
const uint8_t net_key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                             0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

const uint8_t app_key[16] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                             0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};

uint8_t dev_uuid[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

/** Vendor model instance */
esp_ble_mesh_model_t vendor_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, VENDOR_MODEL_ID, op_vendor, NULL, 0,
                              NULL),
};

/** Element */
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, vendor_models, ESP_BLE_MESH_NO_OPS),
};

/** Composition */
static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

/** Provisioning */
static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
};

/** AppKey / NetKey indices */
uint16_t net_idx = NET_IDX;
uint16_t app_idx = APP_IDX;
uint16_t unicast_addr = UNICAST_ADDR;

void mesh_init(void) {
  esp_err_t err;

  ESP_LOGI(TAG, "Initializing BLE Mesh Node...");

  esp_ble_mesh_register_prov_callback(provisioning_cb);
  esp_ble_mesh_register_model_callback(vendor_model_cb);

  esp_ble_mesh_init(&prov, &composition);

  /** Add NetKey */
  err = esp_ble_mesh_provisioner_add_local_net_key(net_key, NET_IDX);
  if (err) {
    ESP_LOGE(TAG, "Failed to add NetKey (err=0x%X)", err);
  }

  /** Add AppKey */
  err = esp_ble_mesh_provisioner_add_local_app_key(app_key, NET_IDX, APP_IDX);
  if (err) {
    ESP_LOGE(TAG, "Failed to add AppKey (err=0x%X)", err);
  }

  /** Bind AppKey to Vendor Model */
  err = esp_ble_mesh_model_bind_app_key(UNICAST_ADDR, NET_IDX, APP_IDX,
                                        &vendor_models[0]);
  if (err) {
    ESP_LOGE(TAG, "Failed to bind AppKey (err=0x%X)", err);
  }

  ESP_LOGI(TAG, "Mesh node initialized with unicast addr 0x%04X", UNICAST_ADDR);
}
