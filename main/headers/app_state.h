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
  int home_selected_option;   // for home screen (chat/about)
  int chat_selected_user;     // for chat list navigation
  int chat_scroll_offset;
  int predefined_selected;
} app_state_t;

// Global instance (defined in app_state.c)
extern app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void);
