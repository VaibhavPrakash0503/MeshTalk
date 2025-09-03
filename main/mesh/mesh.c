#include "mesh.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_log.h"
#include "mesh_init.h"
#include "model_vendor.h"
#include "user_table.h"
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

  if (param->model_operation.opcode == VENDOR_OPCODE_MESSAGE) {
    const uint8_t *data = param->model_operation.msg;
    size_t data_len = param->model_operation.length;

    ESP_LOGI(TAG, "Raw message received, len=%u", (unsigned)data_len);

    // Ensure null-terminated copy for parsing
    char buffer[64];
    if (data_len >= sizeof(buffer)) {
      ESP_LOGW(TAG, "Message too long, truncating");
      data_len = sizeof(buffer) - 1;
    }
    memcpy(buffer, data, data_len);
    buffer[data_len] = '\0';

    // Check for "Name:addr" format
    char *colon = strchr(buffer, ':');
    if (colon) {
      *colon = '\0';
      const char *name = buffer;
      const char *addr_str = colon + 1;

      uint16_t addr =
          (uint16_t)strtol(addr_str, NULL, 0); // auto handles hex/dec
      if (addr != 0) {
        if (user_table_set(name, addr)) {
          ESP_LOGI(TAG, "User table updated -> %s @ 0x%04X", name, addr);
        } else {
          ESP_LOGW(TAG, "User table full or duplicate, could not add %s", name);
        }
      }
      return;
    }

    // Forward raw data to whoever registered
    if (app_receive_cb) {
      uint16_t sender_addr = param->model_operation.ctx->addr;
      app_receive_cb(sender_addr, data, data_len);
    }
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

bool mesh_broadcast_self(void) {
  const node_config_t *cfg = node_config_get();
  if (!cfg || cfg->address == 0x0000 || strlen(cfg->name) == 0) {
    ESP_LOGE("MESH", "Node config not ready, cannot broadcast self");
    return false;
  }

  // Format: "name:0xADDR"
  char announce_msg[64];
  int n = snprintf(announce_msg, sizeof(announce_msg), "%s:0x%04X", cfg->name,
                   cfg->address);
  if (n <= 0) {
    ESP_LOGE("MESH", "Failed to format announce message");
    return false;
  }

  // Prepare broadcast context
  esp_ble_mesh_msg_ctx_t ctx = {0};
  ctx.net_idx = NET_IDX;
  ctx.app_idx = APP_IDX;
  ctx.addr = ESP_BLE_MESH_ADDR_ALL_NODES; // broadcast
  ctx.send_ttl = 5;

  // Send using server model
  esp_err_t err = esp_ble_mesh_server_model_send_msg(
      &vendor_models[0], &ctx, VENDOR_OPCODE_MESSAGE,
      (uint16_t)strlen(announce_msg), (uint8_t *)announce_msg);

  if (err != ESP_OK) {
    ESP_LOGE("MESH", "Broadcast failed (err=0x%X)", err);
    return false;
  }

  ESP_LOGI("MESH", "📢 Broadcasted self: %s", announce_msg);

  // Save ourselves in the user_table too
  user_table_set(cfg->name, cfg->address);

  return true;
}

/* -------------------------------------------------------------------------- */
/*                  Register callback for higher-level app                    */
/* -------------------------------------------------------------------------- */
void mesh_register_receive_cb(mesh_receive_cb_t cb) { app_receive_cb = cb; }
