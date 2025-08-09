#include "ui_screens.h"
#include "ssd1306.h"       // nopnop2002's driver
#include <string.h>

// OLED Device Handle
static SSD1306_t dev;

// UI State
static screen_t current_screen = SCREEN_HOME;
static uint8_t selected_item = 0;
static uint8_t scroll_offset = 0;

// Initialize OLED
void ui_init(void) {
    ssd1306_config_t cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_pin = 21,     // Your SDA GPIO
        .scl_pin = 22,     // Your SCL GPIO
        .screen = SSD1306_SCREEN
    };
    ssd1306_init(&dev, &cfg);
    ssd1306_clear_screen(&dev, false);
}

// Home Screen
static void ui_show_home(void) {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "Mesh Talk", 9, false);
    ssd1306_display_text(&dev, 2, selected_item == 0 ? "> Chat" : "  Chat", 6, false);
    ssd1306_display_text(&dev, 3, selected_item == 1 ? "> About" : "  About", 7, false);
}

// Chat List Screen
static void ui_show_chat_list(void) {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "Contacts    *New", 15, false);
    
    // Example contacts (replace with BLE data later)
    ssd1306_display_text(&dev, 2, selected_item == 0 ? "> User1 ●" : "  User1 ●", 9, false);
    ssd1306_display_text(&dev, 3, selected_item == 1 ? "> User2 ○" : "  User2 ○", 9, false);
}

// Individual Chat Screen
static void ui_show_individual_chat(void) {
    ssd1306_clear_screen(&dev, false);
    
    // Header: [Username] [Online Status]
    ssd1306_display_text(&dev, 0, "User1 ●", 7, false);
    
    // Message History (Example data - replace with BLE messages)
    ssd1306_display_text(&dev, 2, "P: Hello!", 9, false);      // Predefined
    ssd1306_display_text(&dev, 3, "Y: Hi there!", 12, false);  // Your custom
    ssd1306_display_text(&dev, 4, "P: Help", 7, false);        // Predefined
    
    // Input Prompt
    ssd1306_display_text(&dev, 7, "> [custom]", 10, true);  // Inverted
}

// About Screen
static void ui_show_about(void) {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "About", 5, false);
    ssd1306_display_text(&dev, 2, "MeshTalk v1.0", 13, false);
    ssd1306_display_text(&dev, 4, "BLE Mesh Chat", 13, false);
    ssd1306_display_text(&dev, 6, "Team Project", 12, false);
}

// ---- Joystick Handling ----
void ui_handle_joystick(joystick_action_t action) {
    switch (current_screen) {
        case SCREEN_HOME:
            if (action == JOY_DOWN || action == JOY_UP) {
                selected_item = (selected_item + 1) % 2;
            } else if (action == JOY_BTN) {
                current_screen = (selected_item == 0) ? SCREEN_CHAT_LIST : SCREEN_ABOUT;
            }
            break;

        case SCREEN_CHAT_LIST:
            if (action == JOY_DOWN) {
                selected_item = (selected_item + 1) % 2; // 2 example contacts
            } else if (action == JOY_UP) {
                selected_item = (selected_item - 1 + 2) % 2;
            } else if (action == JOY_BTN) {
                current_screen = SCREEN_INDIVIDUAL_CHAT;
            } else if (action == JOY_LEFT) {
                current_screen = SCREEN_HOME;
            }
            break;

        case SCREEN_INDIVIDUAL_CHAT:
            if (action == JOY_LEFT) {
                current_screen = SCREEN_CHAT_LIST;
            } else if (action == JOY_BTN) {
                // (Future: Open message composer)
            }
            break;

        case SCREEN_ABOUT:
            if (action == JOY_LEFT) {
                current_screen = SCREEN_HOME;
            }
            break;
    }
    ui_render_screen(current_screen);
}

// ---- Core UI API ----
void ui_render_screen(screen_t screen) {
    switch (screen) {
        case SCREEN_HOME:          ui_show_home();          break;
        case SCREEN_CHAT_LIST:     ui_show_chat_list();     break;
        case SCREEN_INDIVIDUAL_CHAT: ui_show_individual_chat(); break;
        case SCREEN_ABOUT:         ui_show_about();         break;
        default:                   ui_show_home();          break;
    }
}

// Placeholder for BLE integration
void ui_update_contact_list(const char* contacts[], bool online_status[]) {
}