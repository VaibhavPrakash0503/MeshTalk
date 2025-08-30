#pragma once

#include <stdint.h>

// Function to compute CRC16 checksum using a lookup table
uint16_t crc16(const uint8_t *data, uint16_t length);