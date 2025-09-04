#include "mesh.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_log.h"
#include "mesh_init.h"
#include "model_vendor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "MESH";

static mesh_receive_cb_t app_receive_cb = NULL;

extern esp_ble_mesh_model_t vendor_models[];

/* -------------------------------------------------------------------------- */
/*                   Vendor model callback (receive handler)                  */
/* -------------------------------------------------------------------------- */
void mesh_vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                          esp_ble_mesh_model_cb_param_t *param) {
  if (event != ESP_BLE_MESH_MODEL_OPERATION_EVT) {
    return;
  }

  if (param->model_operation.opcode != VENDOR_OPCODE_MESSAGE) {
    return;
  }

  const uint8_t *data = param->model_operation.msg;
  size_t data_len = param->model_operation.length;

  if (!data || data_len == 0) {
    ESP_LOGW("MESH", "Received empty message");
    return;
  }

  ESP_LOGI("MESH", "Raw message received, len=%u", (unsigned)data_len);

  // Forward raw data to whoever registered
  if (app_receive_cb) {
    app_receive_cb(data, data_len);
  }
}

/* -------------------------------------------------------------------------- */
/*                            Message sending                                 */
/* -------------------------------------------------------------------------- */
int mesh_send_raw(const uint8_t *data, size_t data_len, uint16_t receiver_add) {
  if (!vendor_models[0].model || !data || data_len == 0) {
    ESP_LOGE(TAG, "Invalid arguments to mesh_send_raw");
    return -1;
  }

  esp_ble_mesh_msg_ctx_t ctx = {0};
  ctx.net_idx = NET_IDX;
  ctx.app_idx = APP_IDX;
  ctx.addr = receiver_add;
  ctx.send_rel = true;
  ctx.send_ttl = 5;

  esp_err_t err = esp_ble_mesh_client_model_send_msg(
      &vendor_models[0], &ctx, VENDOR_OPCODE_MESSAGE, data_len, (uint8_t *)data,
      2000, // timeout
      true, // need response
      ROLE_NODE);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Send failed (err=0x%04X)", err);
    return -1;
  }

  ESP_LOGI(TAG, "Raw message sent to 0x%04X, len=%u", receiver_add,
           (unsigned)data_len);
  return 0;
}

bool mesh_broadcast_self(const uint8_t *buffer, size_t buffer_len) {
  if (!buffer || buffer_len == 0) {
    ESP_LOGE("MESH", "Invalid buffer or buffer length");
    return false;
  }

  esp_ble_mesh_msg_ctx_t ctx = {0};
  ctx.net_idx = NET_IDX;
  ctx.app_idx = APP_IDX;
  ctx.addr = 0xFFFF;
  ctx.send_ttl = 5;

  esp_err_t err = esp_ble_mesh_server_model_send_msg(
      &vendor_models[0], &ctx, VENDOR_OPCODE_MESSAGE, (uint16_t)buffer_len,
      (uint8_t *)buffer);

  if (err != ESP_OK) {
    ESP_LOGE("MESH", "Broadcast failed (err=0x%X)", err);
    return false;
  }

  ESP_LOGI("MESH", "📢 Broadcasted self (%d bytes)", buffer_len);
  return true;
}

/* -------------------------------------------------------------------------- */
/*                  Register callback for higher-level app                    */
/* -------------------------------------------------------------------------- */
void mesh_register_receive_cb(mesh_receive_cb_t cb) { app_receive_cb = cb; }
