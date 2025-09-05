#pragma once
#include "constants.h"
#include <stdint.h>

#define MAX_PAYLOAD_SIZE 255

typedef struct {
  uint8_t type;                       // 1 = text 2 = broadcast
  uint64_t timestamp;                 // Time (seconds)
  char sender_name[USERNAME_MAX_LEN]; // sender name
  uint16_t addr;                      // Sender address
  uint8_t payload_len;                // Size of message
  uint8_t payload[MAX_PAYLOAD_SIZE];  // Data
} MeshMessage;
