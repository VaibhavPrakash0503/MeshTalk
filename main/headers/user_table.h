#pragma once
#include <stdint.h>

typedef struct {
  uint16_t user_id;
  const char *username;
  uint16_t unicast_addr; // unicast address
} user_t;

#define MAX_USERS 2

extern user_t user_table[MAX_USERS];

uint16_t get_unicast_addr(uint16_t reciever_id);
