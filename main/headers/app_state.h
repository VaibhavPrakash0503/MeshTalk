#pragma once
#include "constants.h"
#include "types_common.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Global application state
 */
typedef struct {
  screen_t current_screen;
  int selected_user;
  bool new_message_flags[MAX_USERS];
} app_state_t;

// Global instance (defined in app_state.c)
extern app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void);
