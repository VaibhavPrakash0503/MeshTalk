#pragma once

#include <stddef.h>
#include <stdint.h>

// Function to compute CRC16 checksum using a lookup table
uint16_t crc16(const uint8_t *data, size_t length);
