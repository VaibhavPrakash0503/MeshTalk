#include "mesh_init.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "node_config.h"
#include "vendor_model.h"
#include <string.h>

static const char *TAG = "MESH_INIT";

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/* -------------------------------------------------------------------------- */
/*                       UUID & Keys                                          */
/* -------------------------------------------------------------------------- */
uint8_t dev_uuid[16]; // Generated at runtime

/* Hardcoded keys — must match provisioner */
const uint8_t net_key[16] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
                             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};

const uint8_t app_key[16] = {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89,
                             0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89};

static uint8_t static_oob_val[16] = {[0 ... 15] = 0xAA};

/* -------------------------------------------------------------------------- */
/*                  Generate UUID from MAC                                    */
/* -------------------------------------------------------------------------- */
static void generate_uuid_from_mac(void) {
  uint8_t mac[6];
  esp_err_t ret = esp_read_mac(mac, ESP_MAC_WIFI_STA);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get MAC address: %s", esp_err_to_name(ret));
    uint8_t fallback_uuid[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
                                 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                 0x66, 0x77, 0x88, 0x99};
    memcpy(dev_uuid, fallback_uuid, 16);
    return;
  }

  uint8_t base_uuid[12] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
                           0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

  memcpy(dev_uuid, base_uuid, 12);
  memcpy(dev_uuid + 12, mac + 2, 4);

  ESP_LOGI(TAG, "Generated UUID from MAC: %02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/* -------------------------------------------------------------------------- */
/*                       Provisioning & Composition                           */
/* -------------------------------------------------------------------------- */
static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
    .static_val = static_oob_val,
    .static_val_len = sizeof(static_oob_val),
};

extern esp_ble_mesh_model_op_t op_vendor[];
extern void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                            esp_ble_mesh_model_cb_param_t *param);

// static uint16_t vendor_app_idx = APP_IDX;
static esp_ble_mesh_model_pub_t vendor_model_pub = {0};

esp_ble_mesh_model_t vendor_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, VENDOR_MODEL_ID, op_vendor,
                              &vendor_model_pub, NULL),
};

// static esp_ble_mesh_elem_t elements[] = {
//   ESP_BLE_MESH_ELEMENT(0, 0, NULL),
// ESP_BLE_MESH_ELEMENT(0, ARRAY_SIZE(vendor_models), vendor_models),
//};

static esp_ble_mesh_elem_t elements[] = {
    {
        .location = 0,
        .sig_model_count = 0,
        .sig_models = NULL,
        .vnd_model_count = ARRAY_SIZE(vendor_models),
        .vnd_models = vendor_models,
    },
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

/* -------------------------------------------------------------------------- */
/*                     Provisioning Callback                                  */
/* -------------------------------------------------------------------------- */
void provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                     esp_ble_mesh_prov_cb_param_t *param) {
  switch (event) {
  case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
    ESP_LOGI(TAG, "Provisioning component registered");
    break;

  case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT: {
    uint16_t primary_addr = param->node_prov_complete.addr;
    uint16_t net_idx = param->node_prov_complete.net_idx;

    ESP_LOGI(TAG, "✅ Provisioning complete: addr=0x%04X net_idx=0x%03X",
             primary_addr, net_idx);

    // Save to node_config (name must already be set in init)
    node_config_set_address(primary_addr);

    const node_config_t *cfg = node_config_get();
    ESP_LOGI(TAG, "Saved Node Config -> Name: %s, Addr: 0x%04X", cfg->name,
             cfg->address);

    // Add AppKey (normally provisioner handles this)
    esp_err_t err =
        esp_ble_mesh_node_add_local_app_key(app_key, net_idx, APP_IDX);
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "AppKey added locally (AppIdx 0x%03X)", APP_IDX);
    } else {
      ESP_LOGW(TAG, "Add AppKey returned 0x%X (likely already present)", err);
    }

    // Bind AppKey to vendor model
    err = esp_ble_mesh_node_bind_app_key_to_local_model(
        APP_IDX, VENDOR_MODEL_ID, primary_addr, CID_ESP);
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "AppKey bound to vendor model at 0x%04X", primary_addr);
    } else {
      ESP_LOGW(TAG, "Bind AppKey returned 0x%X", err);
    }
    break;
  }

  default:
    ESP_LOGI(TAG, "Unhandled provisioning event %d", event);
    break;
  }
}

/* -------------------------------------------------------------------------- */
/*                          Init Function                                     */
/* -------------------------------------------------------------------------- */
void mesh_init(void) {
  esp_err_t err;

  generate_uuid_from_mac();
  ESP_LOGI(TAG, "Initializing BLE Mesh Node");

  // Init node_config with your chosen name
  node_config_init("MyNode");

  err = esp_ble_mesh_register_prov_callback(provisioning_cb);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Failed to register provisioning callback (err 0x%X)",
             err);
    return;
  }

  err = esp_ble_mesh_register_custom_model_callback(vendor_model_cb);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "⚠️ Failed to register vendor model callback (err 0x%X)", err);
  }

  err = esp_ble_mesh_init(&prov, &composition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Failed to initialize mesh stack (err 0x%X)", err);
    return;
  }

  err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV |
                                      ESP_BLE_MESH_PROV_GATT);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "✅ Provisioning enabled - waiting for provisioner...");
  } else {
    ESP_LOGE(TAG, "❌ Failed to enable provisioning: 0x%X", err);
  }
}
