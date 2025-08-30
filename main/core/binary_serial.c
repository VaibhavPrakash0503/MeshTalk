#include "binary_serial.h"
#include "crc16.h"
#include <string.h>

int serialize_message(const MeshMessage *msg, uint8_t *out_buf,
                      size_t *out_len) {
  if (!msg || !out_buf || !out_len)
    return -1;
  if (msg->payload_len > MAX_PAYLOAD_SIZE)
    return -2;

  size_t idx = 0;

  out_buf[idx++] = msg->type;

  out_buf[idx++] = msg->sender_id & 0xFF;
  out_buf[idx++] = (msg->sender_id >> 8) & 0xFF;

  out_buf[idx++] = msg->receiver_id & 0xFF;
  out_buf[idx++] = (msg->receiver_id >> 8) & 0xFF;

  for (int i = 0; i < 8; ++i) {
    out_buf[idx++] = (msg->timestamp >> (i * 8)) & 0xFF;
  }

  out_buf[idx++] = msg->payload_len;

  memcpy(&out_buf[idx], msg->payload, msg->payload_len);
  idx += msg->payload_len;

  // Append CRC16
  uint16_t crc = crc16(out_buf, idx);
  out_buf[idx++] = (crc >> 8) & 0xFF;
  out_buf[idx++] = crc & 0xFF;

  *out_len = idx;
  return 0;
}

int deserialize_message(const uint8_t *in_buf, size_t in_len,
                        MeshMessage *msg) {
  if (!in_buf || !msg || in_len < 14 + 2)
    return -1;

  size_t idx = 0;

  msg->type = in_buf[idx++];

  msg->sender_id = in_buf[idx++];
  msg->sender_id |= ((uint16_t)in_buf[idx++]) << 8;

  msg->receiver_id = in_buf[idx++];
  msg->receiver_id |= ((uint16_t)in_buf[idx++]) << 8;

  msg->timestamp = 0;
  for (int i = 0; i < 8; ++i) {
    msg->timestamp |= ((uint64_t)in_buf[idx++]) << (i * 8);
  }

  msg->payload_len = in_buf[idx++];
  if (msg->payload_len > MAX_PAYLOAD_SIZE ||
      (idx + msg->payload_len + 2) > in_len)
    return -2;

  memcpy(msg->payload, &in_buf[idx], msg->payload_len);
  idx += msg->payload_len;

  uint16_t recv_crc = ((uint16_t)in_buf[idx] << 8) | in_buf[idx + 1];
  uint16_t calc_crc = crc16_ccitt(in_buf, idx);

  if (recv_crc != calc_crc)
    return -3; // CRC mismatch

  return 0;
}
