#include "app_state.h"
#include <string.h>

// Global state instance
app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void) {
  g_app_state.current_screen = SCREEN_HOME;
  g_app_state.selected_user[0] = '\0';
  memset(g_app_state.new_message_flags, 0,
         sizeof(g_app_state.new_message_flags));
}

// Get current screen
screen_t app_state_get_screen(void) { return g_app_state.current_screen; }

// Set current screen
void app_state_set_screen(screen_t screen) {
  g_app_state.current_screen = screen;
}

// Get selected user
const char *app_state_get_selected_user(void) {
  return g_app_state.selected_user;
}

// Set selected user
void app_state_set_selected_user(const char *username) {
  if (username) {
    strncpy(g_app_state.selected_user, username, USERNAME_MAX_LEN);
    g_app_state.selected_user[USERNAME_MAX_LEN - 1] = '\0';
  } else {
    g_app_state.selected_user[0] = '\0';
  }
}

// Get unread message flag
bool app_state_has_new_message(int user_idx) {
  if (user_idx < 0 || user_idx >= MAX_USERS)
    return false;
  return g_app_state.new_message_flags[user_idx];
}

// Clear unread flag
void app_state_clear_new_message(int user_idx) {
  if (user_idx >= 0 && user_idx < MAX_USERS)
    g_app_state.new_message_flags[user_idx] = false;
}
