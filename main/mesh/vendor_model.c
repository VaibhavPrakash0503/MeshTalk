#include "vendor_model.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_log.h"
#include "mesh.h"

static const char *TAG = "VENDOR_MODEL";

/** Vendor model opcode table */
esp_ble_mesh_model_op_t op_vendor[] = {
    {VENDOR_OPCODE_MESSAGE, 0, 0},
    {VENDOR_OPCODE_ACK, 0, 0},
    ESP_BLE_MESH_MODEL_OP_END,
};

/**
 * @brief Vendor Model callback
 */
void vendor_model_cb(esp_ble_mesh_model_cb_event_t event,
                     esp_ble_mesh_model_cb_param_t *param) {
  if (event == ESP_BLE_MESH_MODEL_OPERATION_EVT) {
    ESP_LOGI(TAG, "Vendor model event, opcode=0x%X",
             param->model_operation.opcode);

    // Just forward everything to mesh.c
    mesh_vendor_model_cb(event, param);
  } else {
    ESP_LOGI(TAG, "Unhandled vendor model event %d", event);
  }
}
