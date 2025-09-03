#pragma once

#include "message_struct.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_SERIALIZED_SIZE (1 + 8 + 1 + MAX_PAYLOAD_SIZE)

int serialize_message(const MeshMessage *msg, uint8_t *out_buf,
                      size_t *out_len);
int deserialize_message(const uint8_t *in_buf, size_t in_len, MeshMessage *msg);
