#include "mesh.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_log.h"
#include "mesh_init.h"
#include "model_vendor.h"

static const char *TAG = "MESH";

static mesh_receive_cb_t app_receive_cb = NULL;

/**
 * @brief Vendor model callback (raw message handler)
 */
void mesh_vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                          esp_ble_mesh_model_cb_param_t *param) {
  if (event != ESP_BLE_MESH_MODEL_OPERATION_EVT) {
    return;
  }

  if (param->model_operation.opcode == VENDOR_OPCODE_MESSAGE) {
    const uint8_t *data = param->model_operation.msg;
    size_t data_len = param->model_operation.length;

    ESP_LOGI(TAG, "Raw message received, len=%u", (unsigned)data_len);

    // Forward raw data to whoever registered
    if (app_receive_cb) {
      app_receive_cb(data, data_len);
    }
  }
}

/**
 * @brief Send a raw buffer over vendor model
 */
int mesh_send_raw(const uint8_t *data, size_t data_len) {
  esp_err_t err = esp_ble_mesh_model_publish(
      vendor_model, VENDOR_OPCODE_MESSAGE, data_len, data, ROLE_NODE);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Send failed (err=0x%04X)", err);
    return -1;
  }

  ESP_LOGI(TAG, "Raw message sent, len=%u", (unsigned)data_len);
  return 0;
}

/**
 * @brief Register a callback for raw incoming data
 */
void mesh_register_receive_cb(mesh_receive_cb_t cb) { app_receive_cb = cb; }
