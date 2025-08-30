#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_log.h"
#include "mesh_init.h"
#include "model_vendor.h"
#include "nvs_flash.h"

#define TAG "MESH_INIT"

static uint8_t dev_uuid[16] = {0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                               0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd};

static const uint8_t net_key[16] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                                    0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
                                    0x1D, 0x1E, 0x1F, 0x20};

static const uint8_t app_key[16] = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
                                    0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
                                    0x2D, 0x2E, 0x2F, 0x30};

static const uint8_t dev_key[16] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
                                    0x3D, 0x3E, 0x3F, 0x40};

#define UNICAST_ADDR 0x0001

static esp_ble_mesh_model_pub_t vendor_model_pub = {
    .msg = NET_BUF_SIMPLE(32),
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(vendor_model_pub_data, 32);

static esp_ble_mesh_model_op_t vendor_ops[] = {
    ESP_BLE_MESH_MODEL_OP(0x00,
                          2), // Chat message opcode (2-byte vendor opcode)
    ESP_BLE_MESH_MODEL_OP_END,
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(),
};

static esp_ble_mesh_model_t vendor_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, VENDOR_MODEL_ID, vendor_ops,
                              &vendor_model_pub, NULL),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, vendor_models),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
};

static void prov_cb(esp_ble_mesh_prov_cb_event_t event,
                    esp_ble_mesh_prov_cb_param_t *param) {
  ESP_LOGI(TAG, "Provisioning event: %d", event);
}

static void config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                             esp_ble_mesh_cfg_server_cb_param_t *param) {
  ESP_LOGI(TAG, "Config Server event: %d", event);
}

static void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                            esp_ble_mesh_model_cb_param_t *param) {
  if (event == ESP_BLE_MESH_MODEL_OPERATION_EVT) {
    ESP_LOGI(TAG, "Received vendor message from 0x%04x",
             param->model_operation.ctx->addr);
    ESP_LOG_BUFFER_HEX("Received", param->model_operation.msg,
                       param->model_operation.length);
  }
}

void mesh_init(void) {
  esp_err_t err;

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_ble_mesh_register_prov_callback(prov_cb));
  ESP_ERROR_CHECK(
      esp_ble_mesh_register_config_server_callback(config_server_cb));
  ESP_ERROR_CHECK(esp_ble_mesh_register_custom_model_callback(vendor_model_cb));

  err = esp_ble_mesh_init(&provision, &composition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ble_mesh_init failed (err 0x%04x)", err);
    return;
  }

  if (!esp_ble_mesh_node_is_provisioned()) {

    err = esp_ble_mesh_node_provision(net_key, NET_IDX, 0, 0, UNICAST_ADDR,
                                      dev_key, 0);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to provision node (err 0x%04x)", err);
      return;
    }

    err = esp_ble_mesh_node_add_local_app_key(app_key, NET_IDX, APP_IDX);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to add local AppKey (err 0x%04x)", err);
      return;
    }

    err = esp_ble_mesh_node_bind_app_key_to_local_model(
        UNICAST_ADDR, 0xFFFF, VENDOR_MODEL_ID, APP_IDX);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to bind AppKey to vendor model (err 0x%04x)", err);
      return;
    }

  } else {
    ESP_LOGI(TAG, "Node already provisioned");
  }

  err = esp_ble_mesh_start();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start BLE Mesh (err 0x%04x)", err);
    return;
  }

  ESP_LOGI(TAG, "BLE Mesh Node initialized");
}
