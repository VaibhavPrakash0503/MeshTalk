#include "app_state.h"

// Define the global instance
app_state_t g_app_state;

/**
 * @brief Initialize the global app state
 */
void app_state_init(void) {
    // Start at home screen
    g_app_state.current_screen = SCREEN_HOME;

    // No user selected initially
    g_app_state.selected_user = 0;

    // Home screen starts with first option ("Chat")
    g_app_state.home_selected_option = 0;

    // Chat list starts at first user
    g_app_state.chat_selected_user = 0;

    g_app_state.chat_scroll_offset = 0;
    
    g_app_state.predefined_selected = 0;

    // Reset message flags
    for (int i = 0; i < MAX_USERS; i++) {
        g_app_state.new_message_flags[i] = false;
    }
}
