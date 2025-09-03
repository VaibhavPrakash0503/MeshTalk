#include "user_table.h"
#include <stdio.h>
#include <string.h>

user_t user_table[MAX_USERS] = {0};

bool user_table_set(const char *username, uint16_t unicast_addr) {
  // Check if address already exists -> do not add
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && user_table[i].unicast_addr == unicast_addr) {
      return false; // Already known
    }
  }

  // Otherwise add a new entry
  for (int i = 0; i < MAX_USERS; i++) {
    if (!user_table[i].valid) {
      user_table[i].valid = true;
      user_table[i].unicast_addr = unicast_addr;
      strncpy(user_table[i].username, username,
              sizeof(user_table[i].username) - 1);
      user_table[i].username[sizeof(user_table[i].username) - 1] = '\0';
      return true;
    }
  }

  // Table full
  return false;
}

uint16_t user_table_get_addr(const char *username) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && strncmp(user_table[i].username, username,
                                       sizeof(user_table[i].username)) == 0) {
      return user_table[i].unicast_addr;
    }
  }
  return 0; // Not found
}

const char *user_table_get_name(uint16_t unicast_addr) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && user_table[i].unicast_addr == unicast_addr) {
      return user_table[i].username;
    }
  }
  return NULL; // Not found
}

void user_table_print(void) {
  printf("=== User Table ===\n");
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid) {
      printf("Name: %s | Addr: 0x%04X\n", user_table[i].username,
             user_table[i].unicast_addr);
    }
  }
}

int user_table_find_index_by_addr(uint16_t addr) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && user_table[i].unicast_addr == addr) {
      return i;
    }
  }
  return -1; // not found
}

int user_table_find_index_by_name(const char *username) {
  if (!username)
    return -1;

  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && strcmp(user_table[i].username, username) == 0) {
      return i;
    }
  }
  return -1; // not found
}
