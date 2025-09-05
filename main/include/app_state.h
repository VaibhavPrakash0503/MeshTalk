#pragma once
#include "constants.h"
#include "types_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Global application state
 */
typedef struct {
  screen_t current_screen;
  char selected_user[USERNAME_MAX_LEN];
  bool new_message_flags[MAX_USERS];
} app_state_t;

// Global instance (defined in app_state.c)
extern app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void);

// Get current screen
screen_t app_state_get_screen(void);

// Set current screen
void app_state_set_screen(screen_t screen);

// Get selected user
const char *app_state_get_selected_user(void);

// Set selected user
void app_state_set_selected_user(const char *username);

// Get unread message flag
bool app_state_has_new_message(int user_idx);

// Clear unread flag
void app_state_clear_new_message(int user_idx);
