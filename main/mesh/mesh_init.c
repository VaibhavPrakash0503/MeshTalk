#include "mesh_init.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
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
/*                       UUID & Storage                                       */
/* -------------------------------------------------------------------------- */
uint8_t dev_uuid[16]; // Generated at runtime
static uint8_t static_oob_val[16] = {[0 ... 15] = 0xAA};

// Store for mesh info (adapted from ESP-IDF example)
static struct mesh_info_store {
  uint16_t net_idx;
  uint16_t app_idx;
  uint16_t node_addr;
  bool provisioned;
} store = {
    .net_idx = ESP_BLE_MESH_KEY_UNUSED,
    .app_idx = ESP_BLE_MESH_KEY_UNUSED,
    .node_addr = 0x0000,
    .provisioned = false,
};

/* -------------------------------------------------------------------------- */
/*                  Generate UUID from MAC (your existing code)              */
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
/*                       Configuration Server */
/* -------------------------------------------------------------------------- */
static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7,
};

/* -------------------------------------------------------------------------- */
/*                       Models & Elements                                    */
/* -------------------------------------------------------------------------- */
// Root models (required for any BLE Mesh node)
static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
};

// External vendor models (from your vendor_model.c)
extern esp_ble_mesh_model_op_t op_vendor[];
extern void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                            esp_ble_mesh_model_cb_param_t *param);

static esp_ble_mesh_model_pub_t vendor_model_pub = {0};

esp_ble_mesh_model_t vendor_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, VENDOR_MODEL_ID, op_vendor,
                              &vendor_model_pub, NULL),
};

// Elements (must include both root and vendor models)
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, vendor_models),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

/* -------------------------------------------------------------------------- */
/*                       Provisioning Structure                               */
/* -------------------------------------------------------------------------- */
static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
    .static_val = static_oob_val,
    .static_val_len = sizeof(static_oob_val),
    .output_size = 0,
    .output_actions = 0,
};

/* -------------------------------------------------------------------------- */
/*                       Provisioning Callbacks                              */
/* -------------------------------------------------------------------------- */
static void meshtalk_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                     esp_ble_mesh_prov_cb_param_t *param) {
  switch (event) {
  case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
    ESP_LOGI(TAG, "✅ BLE Mesh ready for mobile app provisioning");
    break;

  case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
    ESP_LOGI(TAG, "🎯 PROVISIONED by mobile app!");
    ESP_LOGI(TAG, "Net Index: 0x%04x", param->node_prov_complete.net_idx);
    ESP_LOGI(TAG, "Node Address: 0x%04x", param->node_prov_complete.addr);

    // Store provisioning info
    store.net_idx = param->node_prov_complete.net_idx;
    store.node_addr = param->node_prov_complete.addr;
    store.provisioned = true;

    // Update your node config
    node_config_set_address(param->node_prov_complete.addr);

    ESP_LOGI(TAG, "🚀 MeshTalk ready for messaging!");
    break;

  case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
    ESP_LOGI(TAG, "Provisioning enabled - device discoverable by mobile apps");
    break;

  default:
    ESP_LOGD(TAG, "Provisioning event: %d", event);
    break;
  }
}

// Config server callback to detect AppKey binding
static void
meshtalk_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                          esp_ble_mesh_cfg_server_cb_param_t *param) {
  if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
    switch (param->ctx.recv_op) {
    case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
      ESP_LOGI(TAG, "✅ AppKey added by mobile app");
      ESP_LOGI(TAG, "Net IDX: 0x%04x, App IDX: 0x%04x",
               param->value.state_change.appkey_add.net_idx,
               param->value.state_change.appkey_add.app_idx);
      break;

    case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
      ESP_LOGI(TAG, "✅ AppKey bound to model");
      ESP_LOGI(
          TAG,
          "Element: 0x%04x, App IDX: 0x%04x, Company: 0x%04x, Model: 0x%04x",
          param->value.state_change.mod_app_bind.element_addr,
          param->value.state_change.mod_app_bind.app_idx,
          param->value.state_change.mod_app_bind.company_id,
          param->value.state_change.mod_app_bind.model_id);

      // Check if it's our vendor model
      if (param->value.state_change.mod_app_bind.company_id == CID_ESP &&
          param->value.state_change.mod_app_bind.model_id == VENDOR_MODEL_ID) {
        store.app_idx = param->value.state_change.mod_app_bind.app_idx;
        ESP_LOGI(TAG, "🎯 MeshTalk vendor model ready! App IDX: 0x%04x",
                 store.app_idx);
      }
      break;
    }
  }
}

/* -------------------------------------------------------------------------- */
/*                       Public Getter Functions                             */
/* -------------------------------------------------------------------------- */
uint16_t get_net_idx(void) { return store.net_idx; }
uint16_t get_app_idx(void) { return store.app_idx; }
uint16_t get_node_addr(void) { return store.node_addr; }
bool is_provisioned(void) {
  return store.provisioned && store.app_idx != ESP_BLE_MESH_KEY_UNUSED;
}

/* -------------------------------------------------------------------------- */
/*                          Init Function                                     */
/* -------------------------------------------------------------------------- */
void mesh_init(void) {
  esp_err_t err;

  generate_uuid_from_mac();
  ESP_LOGI(TAG, "Initializing MeshTalk BLE Mesh Node");

  // Initialize your node config
  node_config_init("Node 1");

  // Initialize Bluetooth stack (directly here instead of using example
  // function)
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  err = esp_bt_controller_init(&bt_cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Bluetooth controller init failed: %s",
             esp_err_to_name(err));
    return;
  }

  err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Bluetooth controller enable failed: %s",
             esp_err_to_name(err));
    return;
  }

  err = esp_bluedroid_init();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Bluedroid init failed: %s", esp_err_to_name(err));
    return;
  }

  err = esp_bluedroid_enable();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Bluedroid enable failed: %s", esp_err_to_name(err));
    return;
  }

  ESP_LOGI(TAG, "✅ Bluetooth stack initialized");

  // Register callbacks
  esp_ble_mesh_register_prov_callback(meshtalk_provisioning_cb);
  esp_ble_mesh_register_config_server_callback(meshtalk_config_server_cb);
  esp_ble_mesh_register_custom_model_callback(vendor_model_cb);

  // Initialize mesh stack
  err = esp_ble_mesh_init(&provision, &composition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Mesh init failed: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI(TAG, "✅ BLE Mesh stack initialized");

  // Enable provisioning - device becomes discoverable
  err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV |
                                      ESP_BLE_MESH_PROV_GATT);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "❌ Enable provisioning failed: %s", esp_err_to_name(err));
    return;
  }

  ESP_LOGI(TAG, "🔍 MeshTalk device ready for mobile app provisioning!");
  ESP_LOGI(TAG, "📱 Use nRF Mesh or ESP BLE Mesh app to provision this device");
}

void check_provisioning_status(void) {
  if (esp_ble_mesh_node_is_provisioned()) {
    ESP_LOGI(TAG, "✅ Node IS provisioned");
  } else {
    ESP_LOGE(TAG, "❌ Node is NOT provisioned");
  }
}
