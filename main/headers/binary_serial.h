#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAX_PAYLOAD_SIZE 255
#define MAX_SERIALIZED_SIZE (1 + 2 + 2 + 8 + 1 + MAX_PAYLOAD_SIZE + 2)

typedef struct {
  uint8_t type;                      // 1 byte
  uint16_t sender_id;                // 2 bytes
  uint16_t receiver_id;              // 2 bytes
  uint64_t timestamp;                // 8 bytes
  uint8_t payload_len;               // 1 byte
  uint8_t payload[MAX_PAYLOAD_SIZE]; // Compressed/encrypted data
} MeshMessage;

int serialize_message(const MeshMessage *msg, uint8_t *out_buf,
                      size_t *out_len);
int deserialize_message(const uint8_t *in_buf, size_t in_len, MeshMessage *msg);
