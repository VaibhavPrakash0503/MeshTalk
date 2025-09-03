#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char username[32];     // Username of node
  uint16_t unicast_addr; // Mesh unicast address
  bool valid;            // Mark if entry is active
} user_t;

#define MAX_USERS 3 // Maximum known users

extern user_t user_table[MAX_USERS];

/**
 * @brief Add a user to the table if not already present.
 *
 * @param username     Username (string)
 * @param unicast_addr Mesh unicast address
 * @return true if added, false if already exists or table full
 */
bool user_table_set(const char *username, uint16_t unicast_addr);

/**
 * @brief Get unicast address for a given username.
 */
uint16_t user_table_get_addr(const char *username);

/**
 * @brief Get username for a given unicast address.
 */
const char *user_table_get_name(uint16_t unicast_addr);

/**
 * @brief Print all known users (for debugging).
 */
void user_table_print(void);

int user_table_find_index_by_addr(uint16_t addr);
