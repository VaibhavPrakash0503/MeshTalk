#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "joystick.h"
#include "types_common.h"
#include <stdbool.h>
#include <stdint.h>

// UI Configuration
#define UI_PREDEFINED_MSG_COUNT 7
#define UI_BROADCAST_TIMEOUT_MS 3000
#define UI_MESSAGE_SENT_DISPLAY_MS 1000
#define UI_CHAT_HISTORY_LINES 6

// Internal UI state (separate from global app_state)
typedef struct {
  int cursor_pos;
  int scroll_offset;
  bool screen_needs_update;
  bool broadcast_active;
  int64_t broadcast_start_time;
} ui_internal_state_t;

// Core UI functions
void ui_init(void);
void ui_update(void);
void ui_handle_input(void);
void ui_force_refresh(void);

// Screen navigation using types_common.h enums
void ui_set_screen(screen_t screen);
void ui_go_back(void);
screen_t ui_get_current_screen(void);

// Screen rendering functions
void ui_show_home_screen(void);
void ui_show_chat_screen(void);
void ui_show_individual_chat_screen(const char *contact_name);
void ui_show_send_message_screen(void);
void ui_show_broadcast_screen(void);

// Navigation helpers
void ui_navigate_up(void);
void ui_navigate_down(void);
void ui_select_current_item(void);
void ui_send_selected_message(void);

// Joystick handling
void ui_handle_joystick(joystick_action_t action);

// Message callback for API integration
void ui_on_message_received(const char *sender, const char *message);

// State management helpers
bool ui_needs_update(void);
void ui_mark_dirty(void);
void ui_clear_dirty(void);

// Broadcast management
void ui_start_broadcast(void);
void ui_check_broadcast_timeout(void);

// Utility functions
const char *ui_get_predefined_message(int index);
int ui_get_predefined_message_count(void);

