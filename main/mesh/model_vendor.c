#include "model_vendor.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_log.h"

static const char *TAG = "VENDOR_MODEL";

/** Vendor model opcode table */
esp_ble_mesh_model_op_t op_vendor[] = {
    {VENDOR_OPCODE_MESSAGE, 0, NULL},
    {VENDOR_OPCODE_ACK, 0, NULL},
    ESP_BLE_MESH_MODEL_OP_END,
};

/**
 * @brief Vendor Model callback
 */
void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                     esp_ble_mesh_model_cb_param_t *param) {
  switch (event) {
  case ESP_BLE_MESH_MODEL_OPERATION_EVT:
    if (param->model_operation.opcode == VENDOR_OPCODE_MESSAGE) {
      ESP_LOGI(TAG, "Received VENDOR_OPCODE_MESSAGE from 0x%04X",
               param->model_operation.ctx->addr);

      /** Send back ACK */
      esp_ble_mesh_msg_ctx_t ctx = *param->model_operation.ctx;
      uint8_t ack_data[1] = {0xAA}; // dummy payload
      esp_err_t err = esp_ble_mesh_server_model_send_msg(
          param->model_operation.model, &ctx, VENDOR_OPCODE_ACK,
          sizeof(ack_data), ack_data);
      if (err) {
        ESP_LOGE(TAG, "Failed to send ACK (err=0x%X)", err);
      }
    } else if (param->model_operation.opcode == VENDOR_OPCODE_ACK) {
      ESP_LOGI(TAG, "Received VENDOR_OPCODE_ACK from 0x%04X",
               param->model_operation.ctx->addr);
    }
    break;

  default:
    ESP_LOGI(TAG, "Unhandled vendor model event %d", event);
    break;
  }
}

/**
 * @brief Provisioning callback
 */
void provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                     esp_ble_mesh_prov_cb_param_t *param) {
  switch (event) {
  case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
    ESP_LOGI(TAG, "Provisioning component registered");
    break;
  case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
    ESP_LOGI(TAG, "Provisioning complete, addr=0x%04X",
             param->node_prov_complete.addr);
    break;
  default:
    ESP_LOGI(TAG, "Unhandled provisioning event %d", event);
    break;
  }
}
