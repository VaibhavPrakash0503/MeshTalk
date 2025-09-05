#include "ui_screens.h"
#include "display.h"
#include "app_state.h"
#include "constants.h"
#include "api.h"
#include "joystick.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

// UI state for cursor management (using static variables since they're not in app_state)
static int home_cursor = 0;
static int chat_cursor = 0;
static int chat_scroll_offset = 0;
static bool screen_needs_full_refresh = true;
static bool cursor_needs_refresh = true;
static int last_cursor_pos = -1;
static screen_t current_ui_screen = SCREEN_HOME;

// Contact data from API
static char contact_names[MAX_USERS][USERNAME_MAX_LEN];
static int contact_count = 0;

// Predefined messages
const char *predefined_msgs[] = {
    "Hello", "How are you?", "On my way", "Bye!"
};
const int num_predefined_msgs = sizeof(predefined_msgs)/sizeof(predefined_msgs[0]);

// Forward declarations
static void ui_handle_joystick_input(void);
static void ui_update_cursor_only(void);
static void ui_message_received_callback(const char *sender_name, const char *msg);
static void ui_update_contact_list(void);
static void ui_show_broadcasting(void);

void ui_init(void) {
    display_init();
    joystick_init();
    app_state_init();
    
    // Initialize API with message callback
    api_init(ui_message_received_callback);
    
    // Initialize UI state
    home_cursor = 0;
    chat_cursor = 0;
    chat_scroll_offset = 0;
    screen_needs_full_refresh = true;
    cursor_needs_refresh = true;
    current_ui_screen = SCREEN_HOME;
    last_cursor_pos = -1;
    
    // Load contacts from API
    ui_update_contact_list();
    
    ui_render_screen(SCREEN_HOME);
}

void ui_render_screen(screen_t screen) {
    current_ui_screen = screen;
    app_state_set_screen(screen);
    screen_needs_full_refresh = true;
    cursor_needs_refresh = true;
    last_cursor_pos = -1;
    
    // Full screen refresh
    display_clear();
    switch (screen) {
        case SCREEN_HOME: 
            ui_show_home(); 
            break;
        case SCREEN_CHAT: 
            ui_show_chat(); 
            break;
        case SCREEN_ABOUT: 
            ui_show_about(); 
            break;
        default: 
            display_draw_text(0, "Unknown screen"); 
            break;
    }
    display_update();
    screen_needs_full_refresh = false;
    
    // Start input handling loop
    ui_handle_joystick_input();
}

void ui_show_home(void) {
    if (screen_needs_full_refresh) {
        display_draw_text(0, "Home");
        const char *options[] = { "Chat", "Broadcasting", "About" };
        
        for (int i = 0; i < 3; i++) {
            char line[32];
            snprintf(line, sizeof(line), "  %s", options[i]);
            display_draw_text(2 + i, line);
        }
    }
    
    // Update cursor
    if (cursor_needs_refresh) {
        ui_update_cursor_only();
    }
}

void ui_show_chat(void) {
    if (screen_needs_full_refresh) {
        // Update contact list from API
        ui_update_contact_list();
        
        display_draw_text(0, "Contacts");
        
        // Display contacts (3 rows max)
        int display_count = (contact_count > 3) ? 3 : contact_count;
        for (int i = 0; i < display_count; i++) {
            int contact_idx = i + chat_scroll_offset;
            if (contact_idx >= contact_count) break;
            
            char line[32];
            const char *status_symbol = (contact_idx % 2 == 0) ? SYMBOL_ONLINE : SYMBOL_OFFLINE;
            const char *new_msg_symbol = app_state_has_new_message(contact_idx) ? SYMBOL_NEW_MESSAGE : "";
            
            snprintf(line, sizeof(line), "  %s %s %s",
                     status_symbol, 
                     (contact_idx < contact_count) ? contact_names[contact_idx] : "Unknown", 
                     new_msg_symbol);
            display_draw_text(2 + i, line);
        }
    }
    
    // Update cursor
    if (cursor_needs_refresh) {
        ui_update_cursor_only();
    }
}

static void ui_show_broadcasting(void) {
    display_clear();
    display_draw_text(0, "Broadcasting");
    display_draw_text(2, "Broadcasting...");
    display_update();
    
    // Trigger API broadcast
    api_broadcast_addr();
    
    // Show broadcasting message for 2 seconds then go back to home
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ui_render_screen(SCREEN_HOME);
}

void ui_show_about(void) {
    if (screen_needs_full_refresh) {
        display_draw_text(0, "About");
        display_draw_text(2, "  MeshTalk v1.0");
        display_draw_text(3, "  ESP32 BLE Mesh");
    }
    
    // No cursor for about page - it's static
}

// Handle joystick input without full screen refresh
static void ui_handle_joystick_input(void) {
    while (1) {
        joystick_action_t action = joystick_get_action();
        
        if (action == JOY_NONE) {
            vTaskDelay(pdMS_TO_TICKS(50)); // Small delay to prevent overwhelming
            continue;
        }
        
        switch (current_ui_screen) {
            case SCREEN_HOME:
                if (action == JOY_UP && home_cursor > 0) {
                    home_cursor--;
                    cursor_needs_refresh = true;
                } else if (action == JOY_DOWN && home_cursor < 2) {
                    home_cursor++;
                    cursor_needs_refresh = true;
                } else if (action == JOY_BTN) {
                    if (home_cursor == 0) {
                        ui_render_screen(SCREEN_CHAT);
                        return;
                    } else if (home_cursor == 1) {
                        ui_show_broadcasting();
                        return;
                    } else if (home_cursor == 2) {
                        ui_render_screen(SCREEN_ABOUT);
                        return;
                    }
                }
                break;
                
            case SCREEN_CHAT:
                {
                    int max_visible = (contact_count > 3) ? 3 : contact_count;
                    if (action == JOY_UP && chat_cursor > 0) {
                        chat_cursor--;
                        cursor_needs_refresh = true;
                    } else if (action == JOY_DOWN && chat_cursor < max_visible - 1) {
                        chat_cursor++;
                        cursor_needs_refresh = true;
                    } else if (action == JOY_BTN) {
                        int selected_user = chat_cursor + chat_scroll_offset;
                        if (selected_user < contact_count) {
                            app_state_set_selected_user(contact_names[selected_user]);
                            app_state_clear_new_message(selected_user);
                            
                            // Show individual chat (simplified for now)
                            display_clear();
                            display_draw_text(0, contact_names[selected_user]);
                            display_draw_text(2, "  Open Messages");
                            display_draw_text(3, "  Send Message");
                            display_update();
                            
                            // Wait for input and return to contacts
                            while (joystick_get_action() != JOY_LEFT) {
                                vTaskDelay(pdMS_TO_TICKS(100));
                            }
                            
                            ui_render_screen(SCREEN_CHAT);
                            return;
                        }
                    } else if (action == JOY_LEFT) {
                        ui_render_screen(SCREEN_HOME);
                        return;
                    }
                }
                break;
                
            case SCREEN_ABOUT:
                if (action == JOY_LEFT || action == JOY_BTN) {
                    ui_render_screen(SCREEN_HOME);
                    return;
                }
                break;
        }
        
        // Update cursor if needed
        if (cursor_needs_refresh) {
            ui_update_cursor_only();
        }
    }
}

// Update only cursor position without full screen refresh
static void ui_update_cursor_only(void) {
    int current_cursor = 0;
    int line_offset = 2; // Most screens start content at line 2
    
    switch (current_ui_screen) {
        case SCREEN_HOME:
            current_cursor = home_cursor;
            break;
        case SCREEN_CHAT:
            current_cursor = chat_cursor;
            break;
        default:
            return; // No cursor updates needed
    }
    
    // CRITICAL: Only update the specific lines that changed - NO FULL SCREEN REFRESH
    
    if (last_cursor_pos != -1 && last_cursor_pos != current_cursor) {
        // Clear old cursor position by redrawing the line without ">"
        char blank_line[32];

        switch (current_ui_screen) {
            case SCREEN_HOME: {
                const char *options[] = { "Chat", "Broadcasting", "About" };
                snprintf(blank_line, sizeof(blank_line), "  %s", options[last_cursor_pos]);
                display_draw_text(line_offset + last_cursor_pos, blank_line);
                break;
            }
            case SCREEN_CHAT: {
                int idx = last_cursor_pos + chat_scroll_offset;
                if (idx < contact_count) {
                    const char *status_symbol = (idx % 2 == 0) ? SYMBOL_ONLINE : SYMBOL_OFFLINE;
                    const char *new_msg_symbol = app_state_has_new_message(idx) ? SYMBOL_NEW_MESSAGE : "";
                    snprintf(blank_line, sizeof(blank_line), "  %s %s %s", status_symbol, contact_names[idx], new_msg_symbol);
                    display_draw_text(line_offset + last_cursor_pos, blank_line);
                }
                break;
            }
        }
    }
    
    // Draw new cursor position with ">"
    char cursor_line[32];
    
    switch (current_ui_screen) {
        case SCREEN_HOME: {
            const char *options[] = { "Chat", "Broadcasting", "About" };
            snprintf(cursor_line, sizeof(cursor_line), "> %s", options[current_cursor]);
            display_draw_text(line_offset + current_cursor, cursor_line);
            break;
        }
        case SCREEN_CHAT: {
            int idx = current_cursor + chat_scroll_offset;
            if (idx < contact_count) {
                const char *status_symbol = (idx % 2 == 0) ? SYMBOL_ONLINE : SYMBOL_OFFLINE;
                const char *new_msg_symbol = app_state_has_new_message(idx) ? SYMBOL_NEW_MESSAGE : "";
                snprintf(cursor_line, sizeof(cursor_line), "> %s %s %s", status_symbol, contact_names[idx], new_msg_symbol);
                display_draw_text(line_offset + current_cursor, cursor_line);
            }
            break;
        }
    }

    display_update();

    last_cursor_pos = current_cursor;
    cursor_needs_refresh = false;
}

static void ui_update_contact_list(void) {
    contact_count = api_get_contact_names(contact_names, MAX_USERS);
}

static void ui_message_received_callback(const char *sender_name, const char *msg) {
    // Update contact list and refresh if on chat screen
    ui_update_contact_list();
    
    if (current_ui_screen == SCREEN_CHAT) {
        screen_needs_full_refresh = true;
        ui_show_chat();
        display_update();
        screen_needs_full_refresh = false;
    }
}
