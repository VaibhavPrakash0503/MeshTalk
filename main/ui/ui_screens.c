#include "ui_screens.h"
#include "api.h"
#include "app_state.h"
#include "constants.h"
#include "display.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "types_common.h"
#include "user_table.h"
#include <string.h>

static const char *TAG = "UI_SCREENS";

// Pre-defined message options
static const char *predefined_messages[UI_PREDEFINED_MSG_COUNT] = {
    "Hello", "How are you?", "Thanks", "Yes", "No", "OK", "Goodbye"};

// Internal UI state (separate from app_state)
static ui_internal_state_t ui_internal = {.cursor_pos = 0,
                                          .scroll_offset = 0,
                                          .screen_needs_update = true,
                                          .broadcast_active = false,
                                          .broadcast_start_time = 0};

// Screen data using constants
static char contact_list[MAX_USERS][USERNAME_MAX_LEN];
static int contact_count = 0;
static char chat_messages[50][MAX_MESSAGE_LEN + USERNAME_MAX_LEN + 10];
static int message_count = 0;

void ui_init(void) {
  ESP_LOGI(TAG, "Initializing MeshTalk UI...");

  api_init(ui_on_message_received);

  // Use app_state for initial screen
  app_state_set_screen(SCREEN_HOME);
  ui_internal.screen_needs_update = true;

  ESP_LOGI(TAG, "✅ MeshTalk UI initialized");
}

void ui_set_screen(screen_t screen) {
  // Use app_state for screen management
  app_state_set_screen(screen);
  ui_internal.cursor_pos = 0;
  ui_internal.scroll_offset = 0;
  ui_internal.screen_needs_update = true;

  // Clear new message flag when opening individual chat
  if (screen == SCREEN_INDIVIDUAL_CHAT) {
    const char *selected_user = app_state_get_selected_user();
    if (selected_user[0] != '\0') {
      int user_idx = user_table_find_index_by_name(selected_user);
      if (user_idx >= 0) {
        app_state_clear_new_message(user_idx);
        ESP_LOGI(TAG, "Cleared new message flag for %s", selected_user);
      }
    }
  }

  ESP_LOGI(TAG, "Screen changed to: %d", screen);
}

void ui_check_broadcast_timeout(void) {
  // Check if we're in broadcast mode
  if (app_state_get_screen() != SCREEN_BROADCAST ||
      !ui_internal.broadcast_active) {
    return;
  }

  // Calculate elapsed time since broadcast started
  int64_t elapsed_ms =
      (esp_timer_get_time() - ui_internal.broadcast_start_time) / 1000;

  // Check if timeout has been reached
  if (elapsed_ms > UI_BROADCAST_TIMEOUT_MS) {
    // Timeout reached - stop broadcast and return to home
    ui_internal.broadcast_active = false;
    ui_set_screen(SCREEN_HOME);
    ESP_LOGI(TAG, "Broadcast timeout after %lld ms - returning to home",
             elapsed_ms);
  }
}

// Also add the missing ui_start_broadcast() function
void ui_start_broadcast(void) {
  ui_internal.broadcast_active = true;
  ui_internal.broadcast_start_time = esp_timer_get_time();

  ESP_LOGI(TAG, "Starting broadcast mode with %d ms timeout",
           UI_BROADCAST_TIMEOUT_MS);

  // The broadcast screen will be shown by ui_set_screen call in
  // ui_select_current_item
}

void ui_go_back(void) {
  screen_t current = app_state_get_screen();

  switch (current) {
  case SCREEN_CHAT:
    ui_set_screen(SCREEN_HOME);
    break;
  case SCREEN_INDIVIDUAL_CHAT:
    ui_set_screen(SCREEN_CHAT);
    break;
  case SCREEN_SEND_MESSAGE:
    ui_set_screen(SCREEN_INDIVIDUAL_CHAT);
    break;
  case SCREEN_BROADCAST:
    ui_set_screen(SCREEN_HOME);
    break;
  default:
    break;
  }
}

screen_t ui_get_current_screen(void) { return app_state_get_screen(); }

// HOME SCREEN (Menu mode)
void ui_show_home_screen(void) {
  const char *menu_items[] = {"Chat", "Broadcast"};

  display_set_mode(DISPLAY_MODE_MENU);
  display_show_menu_screen("MeshTalk", menu_items, 2, ui_internal.cursor_pos);

  ESP_LOGD(TAG, "Home screen displayed, cursor at %d", ui_internal.cursor_pos);
}

// CHAT LIST SCREEN with new message indicators
void ui_show_chat_screen(void) {
  contact_count = api_get_contact_names(contact_list, MAX_USERS);

  if (contact_count == 0) {
    const char *empty_list[] = {"No contacts found"};
    display_set_mode(DISPLAY_MODE_LIST);
    display_show_list_screen("Chat", empty_list, 1, 0, 0);
    return;
  }

  // Create contact display names with new message indicators using
  // SYMBOL_NEW_MESSAGE
  static char display_names[MAX_USERS][USERNAME_MAX_LEN + 5];
  for (int i = 0; i < contact_count; i++) {
    int user_idx = user_table_find_index_by_name(contact_list[i]);
    if (user_idx >= 0 && app_state_has_new_message(user_idx)) {
      snprintf(display_names[i], sizeof(display_names[i]), "%s %s",
               contact_list[i], SYMBOL_NEW_MESSAGE);
    } else {
      strncpy(display_names[i], contact_list[i], sizeof(display_names[i]) - 1);
      display_names[i][sizeof(display_names[i]) - 1] = '\0';
    }
  }

  // Calculate scroll offset for smooth scrolling
  int visible_items = 7;
  if (ui_internal.cursor_pos >= ui_internal.scroll_offset + visible_items) {
    ui_internal.scroll_offset = ui_internal.cursor_pos - visible_items + 1;
  } else if (ui_internal.cursor_pos < ui_internal.scroll_offset) {
    ui_internal.scroll_offset = ui_internal.cursor_pos;
  }

  const char *contact_names[MAX_USERS];
  for (int i = 0; i < contact_count; i++) {
    contact_names[i] = display_names[i];
  }

  display_set_mode(DISPLAY_MODE_LIST);
  display_show_list_screen("Chat", contact_names, contact_count,
                           ui_internal.cursor_pos, ui_internal.scroll_offset);

  ESP_LOGD(TAG, "Chat screen displayed: %d contacts", contact_count);
}

// INDIVIDUAL CHAT SCREEN
void ui_show_individual_chat_screen(const char *contact_name) {
  static char raw_messages[50][MAX_MESSAGE_LEN + 1];
  message_count = api_get_chat_log(contact_name, raw_messages, 50);

  // Safe message formatting with null termination
  for (int i = 0; i < message_count; i++) {
    strncpy(chat_messages[i], raw_messages[i], sizeof(chat_messages[i]) - 1);
    chat_messages[i][sizeof(chat_messages[i]) - 1] = '\0';
  }

  display_set_mode(DISPLAY_MODE_CHAT);
  display_clear();

  // Header with contact name
  display_center_text(0, contact_name, false);

  // Show recent messages (lines 1-6)
  int start_msg = (message_count > UI_CHAT_HISTORY_LINES)
                      ? message_count - UI_CHAT_HISTORY_LINES
                      : 0;

  for (int i = 0; i < UI_CHAT_HISTORY_LINES; i++) {
    int msg_index = start_msg + i;
    if (msg_index < message_count) {
      display_list_line(i + 1, chat_messages[msg_index], false);
    } else {
      display_list_line(i + 1, "", false);
    }
  }

  // Show "Send Message" option on line 7
  bool send_selected = (ui_internal.cursor_pos == 0);
  display_list_line(7, "Send Message", send_selected);

  ESP_LOGD(TAG, "Individual chat displayed: %s, %d messages", contact_name,
           message_count);
}

// SEND MESSAGE SCREEN
void ui_show_send_message_screen(void) {
  display_set_mode(DISPLAY_MODE_LIST);
  display_clear();

  // Header
  char header[32];
  const char *selected_user = app_state_get_selected_user();
  snprintf(header, sizeof(header), "Send to %s", selected_user);
  display_center_text(0, header, false);

  // Show message options (lines 1-7)
  int visible_options = 7;
  for (int i = 0; i < visible_options && i < UI_PREDEFINED_MSG_COUNT; i++) {
    int option_index = ui_internal.scroll_offset + i;
    if (option_index < UI_PREDEFINED_MSG_COUNT) {
      bool is_selected = (option_index == ui_internal.cursor_pos);
      display_list_line(i + 1, predefined_messages[option_index], is_selected);
    } else {
      display_list_line(i + 1, "", false);
    }
  }

  ESP_LOGD(TAG, "Send message screen displayed, cursor at %d",
           ui_internal.cursor_pos);
}

// BROADCAST SCREEN
void ui_show_broadcast_screen(void) {
  display_set_mode(DISPLAY_MODE_MENU);
  display_clear();

  display_center_text(1, "Broadcasting...", true);
  display_center_text(2, "Please wait", false);

  ESP_LOGD(TAG, "Broadcast screen displayed");
}

void ui_update(void) {
  if (!ui_internal.screen_needs_update)
    return;

  screen_t current = app_state_get_screen();
  switch (current) {
  case SCREEN_HOME:
    ui_show_home_screen();
    break;
  case SCREEN_CHAT:
    ui_show_chat_screen();
    break;
  case SCREEN_INDIVIDUAL_CHAT:
    ui_show_individual_chat_screen(app_state_get_selected_user());
    break;
  case SCREEN_SEND_MESSAGE:
    ui_show_send_message_screen();
    break;
  case SCREEN_BROADCAST:
    ui_show_broadcast_screen();
    break;
  }

  ui_internal.screen_needs_update = false;
}

void ui_navigate_up(void) {
  if (ui_internal.cursor_pos > 0) {
    ui_internal.cursor_pos--;
    ui_internal.screen_needs_update = true;
  }
}

void ui_navigate_down(void) {
  int max_pos = 0;
  screen_t current = app_state_get_screen();

  switch (current) {
  case SCREEN_HOME:
    max_pos = 1; // Chat, Broadcast
    break;
  case SCREEN_CHAT:
    max_pos = contact_count - 1;
    break;
  case SCREEN_INDIVIDUAL_CHAT:
    max_pos = 0; // Only Send Message option
    break;
  case SCREEN_SEND_MESSAGE:
    max_pos = UI_PREDEFINED_MSG_COUNT - 1;
    break;
  case SCREEN_BROADCAST: // Handle broadcast case
    max_pos = 0;         // No navigation during broadcast
    break;
  default: // Add default case to fix clangd error
    ESP_LOGW(TAG, "Unknown screen in navigate_down: %d", current);
    max_pos = 0;
    break;
  }

  if (ui_internal.cursor_pos < max_pos) {
    ui_internal.cursor_pos++;
    ui_internal.screen_needs_update = true;
  }
}

void ui_select_current_item(void) {
  screen_t current = app_state_get_screen();

  switch (current) {
  case SCREEN_HOME:
    if (ui_internal.cursor_pos == 0) { // Chat
      ui_set_screen(SCREEN_CHAT);
    } else if (ui_internal.cursor_pos == 1) { // Broadcast
      api_broadcast_addr();
      ui_internal.broadcast_active = true;
      ui_internal.broadcast_start_time = esp_timer_get_time();
      ui_set_screen(SCREEN_BROADCAST);
    }
    break;

  case SCREEN_CHAT:
    if (contact_count > 0 && ui_internal.cursor_pos < contact_count) {
      // Use app_state to set selected user
      app_state_set_selected_user(contact_list[ui_internal.cursor_pos]);
      ui_set_screen(SCREEN_INDIVIDUAL_CHAT);
    }
    break;

  case SCREEN_INDIVIDUAL_CHAT:
    if (ui_internal.cursor_pos == 0) { // Send Message option
      ui_set_screen(SCREEN_SEND_MESSAGE);
    }
    break;

  case SCREEN_SEND_MESSAGE:
    ui_send_selected_message();
    break;
  case SCREEN_BROADCAST: // No user interaction during broadcast - just showing
                         // status
    break;
  default:
    ESP_LOGW(TAG, "Unknown screen in select_current_item: %d", current);
    break;
  }
}

void ui_send_selected_message(void) {
  if (ui_internal.cursor_pos < UI_PREDEFINED_MSG_COUNT) {
    const char *message = predefined_messages[ui_internal.cursor_pos];
    const char *selected_user = app_state_get_selected_user();

    ESP_LOGI(TAG, "Sending message '%s' to %s", message, selected_user);

    // Call API to send message
    api_send_text(selected_user, message);

    // Show confirmation
    display_clear();
    display_center_text(2, "Message Sent!", true);
    vTaskDelay(pdMS_TO_TICKS(UI_MESSAGE_SENT_DISPLAY_MS));

    ui_set_screen(SCREEN_INDIVIDUAL_CHAT);
  }
}

void ui_handle_joystick(joystick_action_t action) {
  switch (action) {
  case JOY_UP:
    ui_navigate_up();
    break;
  case JOY_DOWN:
    ui_navigate_down();
    break;
  case JOY_LEFT:
    ESP_LOGI(TAG, "LEFT detected - going back from screen %d",
             app_state_get_screen()); // Go back
    ui_go_back();
    break;
  case JOY_RIGHT:
  case JOY_BTN: // Select option
    ui_select_current_item();
    break;
  case JOY_NONE:
    break;
  }
}

// MESSAGE CALLBACK with app_state integration
void ui_on_message_received(const char *sender, const char *message) {
  ESP_LOGI(TAG, "New message from %s: %s", sender, message);

  const char *current_user = app_state_get_selected_user();

  // If viewing this contact's chat, refresh and clear flag
  if (app_state_get_screen() == SCREEN_INDIVIDUAL_CHAT &&
      strcmp(current_user, sender) == 0) {
    ui_internal.screen_needs_update = true;
    int user_idx = user_table_find_index_by_name(sender);
    if (user_idx >= 0) {
      app_state_clear_new_message(user_idx);
    }
  } else {
    // Set new message flag
    int user_idx = user_table_find_index_by_name(sender);
    if (user_idx >= 0) {
      g_app_state.new_message_flags[user_idx] = true;
    }
  }

  // Refresh chat list if viewing it
  if (app_state_get_screen() == SCREEN_CHAT) {
    ui_internal.screen_needs_update = true;
  }
}

// Utility functions
bool ui_needs_update(void) { return ui_internal.screen_needs_update; }

void ui_mark_dirty(void) { ui_internal.screen_needs_update = true; }

void ui_clear_dirty(void) { ui_internal.screen_needs_update = false; }

const char *ui_get_predefined_message(int index) {
  if (index >= 0 && index < UI_PREDEFINED_MSG_COUNT) {
    return predefined_messages[index];
  }
  return NULL;
}

int ui_get_predefined_message_count(void) { return UI_PREDEFINED_MSG_COUNT; }
