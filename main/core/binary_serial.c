#include "binary_serial.h"
#include <string.h>

int serialize_message(const MeshMessage *msg, uint8_t *out_buf,
                      size_t *out_len) {
  if (!msg || !out_buf || !out_len)
    return -1;

  if (msg->payload_len > MAX_PAYLOAD_SIZE)
    return -2;

  size_t idx = 0;

  out_buf[idx++] = msg->type;

  for (int i = 0; i < 8; ++i) {
    out_buf[idx++] = (msg->timestamp >> (i * 8)) & 0xFF;
  }
  // sender_name (fixed length)
  memcpy(&out_buf[idx], msg->sender_name, USERNAME_MAX_LEN);
  idx += USERNAME_MAX_LEN;

  // addr (big-endian)
  out_buf[idx++] = (msg->addr >> 8) & 0xFF;
  out_buf[idx++] = msg->addr & 0xFF;

  out_buf[idx++] = msg->payload_len;

  if (msg->payload_len > 0) {
    memcpy(&out_buf[idx], msg->payload, msg->payload_len);
    idx += msg->payload_len;
  }

  *out_len = idx;
  return 0;
}

int deserialize_message(const uint8_t *in_buf, size_t in_len,
                        MeshMessage *msg) {
  if (!in_buf || !msg || in_len < (1 + 8 + USERNAME_MAX_LEN + 2 + 1))
    return -1;

  size_t idx = 0;

  // type
  msg->type = in_buf[idx++];

  // timestamp
  msg->timestamp = 0;
  for (int i = 0; i < 8; ++i) {
    msg->timestamp |= ((uint64_t)in_buf[idx++]) << (i * 8);
  }

  // sender_name
  memcpy(msg->sender_name, &in_buf[idx], USERNAME_MAX_LEN);
  idx += USERNAME_MAX_LEN;
  msg->sender_name[USERNAME_MAX_LEN - 1] = '\0';

  // addr
  msg->addr = ((uint16_t)in_buf[idx] << 8) | in_buf[idx + 1];
  idx += 2;
  // payload_len
  msg->payload_len = in_buf[idx++];
  if (msg->payload_len > MAX_PAYLOAD_SIZE || (idx + msg->payload_len) > in_len)
    return -2;

  // payload
  if (msg->payload_len > 0) {
    memcpy(msg->payload, &in_buf[idx], msg->payload_len);
  } else {
    memset(msg->payload, 0, MAX_PAYLOAD_SIZE);
  }

  return 0;
}
