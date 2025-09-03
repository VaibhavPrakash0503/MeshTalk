#include "ui_screens.h"
#include "display.h"
#include "app_state.h"
#include "constants.h"
#include <stdio.h>
#include <string.h>

static const char *dummy_chat_log[CHAT_LOG_SIZE] = {
    "Hi!", "Hello :)", "How are you?", "I'm good!",
    "Testing MeshTalk", "Cool project!", "Let's meet",
    "Tomorrow?", "Sure, 5pm", "Ok, see you!", "Bye!",
    "Take care", "Mesh rocks", "End of log", "..."
};

const char *predefined_msgs[] = {
    "Hello", "How are you?", "On my way", "Bye!"
};
const int num_predefined_msgs = sizeof(predefined_msgs)/sizeof(predefined_msgs[0]);

void ui_init(void) {
    display_init();
    app_state_init();
    ui_render_screen(SCREEN_HOME);
}

void ui_render_screen(screen_t screen) {
    display_clear();
    switch (screen) {
        case SCREEN_HOME: ui_show_home(); break;
        case SCREEN_CHAT: ui_show_chat(); break;
        case SCREEN_ABOUT: ui_show_about(); break;
        default: display_draw_text(0, "Unknown screen"); break;
    }
    display_update();
}

void ui_show_home(void) {
    display_draw_text(0, "Home Screen");
    const char *options[] = { "Chat", "About" };
    for (int i = 0; i < 2; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%s %s",
                 (i == g_app_state.home_selected_option) ? ">" : " ",
                 options[i]);
        display_draw_text(2 + i, line);
    }
}

void ui_show_chat(void) {
    display_draw_text(0, "Contacts");
    for (int i = 0; i < MAX_USERS; i++) {
        char line[32];
        const char *status_symbol = (i % 2 == 0) ? SYMBOL_ONLINE : SYMBOL_OFFLINE;
        const char *new_msg_symbol = g_app_state.new_message_flags[i] ? SYMBOL_NEW_MESSAGE : "";
        snprintf(line, sizeof(line), "%s [%s] User%d %s",
                 (i == g_app_state.chat_selected_user) ? ">" : " ",
                 status_symbol, i + 1, new_msg_symbol);
        display_draw_text(2 + i, line);
    }
}

void ui_show_individual_chat(int user_id) {
    char heading[32];
    snprintf(heading, sizeof(heading), "Chat: User%d", user_id + 1);
    display_draw_text(0, heading);
    for (int i = 0; i < 4; i++) {
        int msg_index = g_app_state.chat_scroll_offset + i;
        if (msg_index < CHAT_LOG_SIZE)
            display_draw_text(2 + i, dummy_chat_log[msg_index]);
    }
    display_draw_text(7, "> Send msg...");
}

void ui_show_send_message_menu(int user_id) {
    char heading[32];
    snprintf(heading, sizeof(heading), "Send to User%d", user_id + 1);
    display_draw_text(0, heading);
    for (int i = 0; i < num_predefined_msgs; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%s %s",
                 (i == g_app_state.predefined_selected) ? ">" : " ",
                 predefined_msgs[i]);
        display_draw_text(2 + i, line);
    }
}

void ui_show_about(void) {
    display_draw_text(0, "About MeshTalk");
    display_draw_text(2, "ESP32 BLE Mesh");
    display_draw_text(4, "< Back");
}
