#pragma once
#include "types_common.h"
#include "constants.h"
#include "joystick.h"  // For joystick_action_t

typedef enum {
    SCREEN_HOME,           ///< Home screen with main menu
    SCREEN_CHAT_LIST,      ///< List of chat contacts
    SCREEN_INDIVIDUAL_CHAT,///< 1:1 chat interface
    SCREEN_ABOUT,          ///< System info screen
    SCREEN_NEW_MESSAGE     ///< Message input screen (stub)
} screen_t;

void ui_init(void);
void ui_render_screen(screen_t screen);
void ui_handle_joystick(joystick_action_t action);
void ui_update_contact_list(const char* contacts[], bool online_status[]);

// Helper Functions (Optional)
screen_t ui_get_current_screen(void);  // Add this if needed for BLE integration