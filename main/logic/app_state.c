#include "app_state.h"
#include <string.h>

// Global state instance
app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void) {
  g_app_state.current_screen = SCREEN_HOME;
  g_app_state.selected_user = 0x0000;
  memset(g_app_state.new_message_flags, 0,
         sizeof(g_app_state.new_message_flags));
}
