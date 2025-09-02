#pragma once
#include <stdint.h>

#define MAX_PAYLOAD_SIZE 255
#define MAX_MESSAGE_LEN 100

typedef struct {
  uint8_t type;                      // 1 = text
  uint16_t sender_id;                // Who sent it
  uint16_t receiver_id;              // Who should get it
  uint64_t timestamp;                // Time (seconds)
  uint8_t payload_len;               // Size of message
  uint8_t payload[MAX_PAYLOAD_SIZE]; // Data
} MeshMessage;
