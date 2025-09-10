#include "message_handler.h"
#include "binary_serial.h"
#include "crc16.h"
#include "mesh.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static app_receive_cb_t app_receive_cb = NULL; // app-level callback

static const char *TAG = "Message_handler";
/**
 * @brief Initialize message handler and hook into mesh
 */
void message_handler_init(void) {
  // Register our raw receive handler with mesh
  mesh_register_receive_cb(message_handler_receive_raw);
}

/**
 * @brief Called by mesh.c whenever a raw buffer arrives
 */
void message_handler_receive_raw(const uint8_t *data, size_t len) {

  ESP_LOGI(TAG, "Message recieved from Mesh");
  if (!data || len < 3) {
    return;
  }

  MeshMessage msg;
  int ret = message_handler_process_incoming(data, len, &msg);
  if (ret == 0) {
    // Pass processed message to application Api
    if (app_receive_cb) {
      ESP_LOGI(TAG, "Message sent to API");
      app_receive_cb(&msg);
    }
  }
}

/**
 * @brief Fully process an incoming raw buffer
 * - Verify CRC
 * - Deserialize into MeshMessage
 */
int message_handler_process_incoming(const uint8_t *data, size_t len,
                                     MeshMessage *out_msg) {

  if (!data || len < 3 || !out_msg) {
    return -1;
  }

  // Extract CRC16 (last two bytes)
  uint16_t recv_crc = ((uint16_t)data[len - 2] << 8) | data[len - 1];
  uint16_t calc_crc = crc16(data, len - 2);

  if (recv_crc != calc_crc) {
    printf("CRC mismatch: recv=0x%04X calc=0x%04X\n", recv_crc, calc_crc);
    return -2;
  }

  // Deserialize (excluding CRC bytes)
  int ret = deserialize_message(data, len - 2, out_msg);
  if (ret != 0) {
    return -3;
  }
  ESP_LOGI(TAG, "incoming message handled success");
  return 0;
}

/**
 * @brief Send a processed MeshMessage
 * - Serialize
 * - Append CRC
 * - Forward to mesh.c
 */
int message_handler_send(const MeshMessage *msg, uint16_t receiver_add) {

  ESP_LOGI(TAG, "Message recieved from API");
  if (!msg) {
    return -1;
  }

  uint8_t buffer[sizeof(MeshMessage) + 4];
  size_t buffer_len = 0;

  // Serialize (without CRC)
  int ret = serialize_message(msg, buffer, &buffer_len);
  if (ret != 0) {
    return -2;
  }

  // Append CRC16
  uint16_t crc = crc16(buffer, buffer_len);
  buffer[buffer_len++] = (crc >> 8) & 0xFF;
  buffer[buffer_len++] = crc & 0xFF;

  ESP_LOGI(TAG, "Message send to mesh");
  // Pass to mesh for transport
  return mesh_send_raw(buffer, buffer_len, receiver_add);
}

void message_handler_broadcast(MeshMessage *msg) {

  ESP_LOGI(TAG, "Broadcast recieved from API");

  uint8_t buffer[sizeof(MeshMessage) + 4];
  size_t buffer_len = 0;

  // Serialize (without CRC)
  int ret = serialize_message(msg, buffer, &buffer_len);

  // Append CRC16
  uint16_t crc = crc16(buffer, buffer_len);
  buffer[buffer_len++] = (crc >> 8) & 0xFF;
  buffer[buffer_len++] = crc & 0xFF;

  // Pass to mesh for transport
  mesh_broadcast_self(buffer, buffer_len);
}

/**
 * @brief Register application callback for processed messages
 */
void message_handler_register_app_cb(app_receive_cb_t cb) {
  app_receive_cb = cb;
}
