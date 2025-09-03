#include "ui_screens.h"
#include "joystick.h"
#include "app_state.h"
#include "constants.h"
#include "api.h"
#include <stdio.h>

typedef enum {
    STATE_HOME,
    STATE_CHAT_LIST,
    STATE_INDIVIDUAL_CHAT,
    STATE_SEND_MENU,
    STATE_ABOUT
} ui_state_t;

static ui_state_t ui_state = STATE_HOME;

void ui_loop(void) {
    joystick_action_t action = joystick_get_action();

    switch (ui_state) {
        case STATE_HOME:
            if (action == JOY_UP && g_app_state.home_selected_option > 0)
                g_app_state.home_selected_option--;
            if (action == JOY_DOWN && g_app_state.home_selected_option < 1)
                g_app_state.home_selected_option++;
            if (action == JOY_BTN) {
                ui_state = (g_app_state.home_selected_option == 0) ? STATE_CHAT_LIST : STATE_ABOUT;
            }
            ui_show_home();
            break;

        case STATE_CHAT_LIST:
            if (action == JOY_UP && g_app_state.chat_selected_user > 0)
                g_app_state.chat_selected_user--;
            if (action == JOY_DOWN && g_app_state.chat_selected_user < MAX_USERS - 1)
                g_app_state.chat_selected_user++;
            if (action == JOY_BTN) {
                g_app_state.selected_user = g_app_state.chat_selected_user;
                ui_state = STATE_INDIVIDUAL_CHAT;
            }
            if (action == JOY_LEFT) {
                ui_state = STATE_HOME;
            }
            ui_show_chat();
            display_update();
            break;

        case STATE_INDIVIDUAL_CHAT:
            if (action == JOY_UP && g_app_state.chat_scroll_offset > 0)
                g_app_state.chat_scroll_offset--;
            if (action == JOY_DOWN && g_app_state.chat_scroll_offset < CHAT_LOG_SIZE - 4)
                g_app_state.chat_scroll_offset++;
            if (action == JOY_BTN) {
                ui_state = STATE_SEND_MENU;
            }
            if (action == JOY_LEFT) {
                ui_state = STATE_CHAT_LIST;
            }
            ui_show_individual_chat(g_app_state.selected_user);
            display_update();
            break;

        case STATE_SEND_MENU:
            if (action == JOY_UP && g_app_state.predefined_selected > 0)
                g_app_state.predefined_selected--;
            if (action == JOY_DOWN && g_app_state.predefined_selected < 3)
                g_app_state.predefined_selected++;
            if (action == JOY_BTN) {
                api_send_text(g_app_state.selected_user,
                              predefined_msgs[g_app_state.predefined_selected]);
                ui_state = STATE_INDIVIDUAL_CHAT;
            }
            if (action == JOY_LEFT) {
                ui_state = STATE_INDIVIDUAL_CHAT;
            }
            ui_show_send_message_menu(g_app_state.selected_user);
            display_update();
            break;

        case STATE_ABOUT:
            if (action == JOY_LEFT)
                ui_state = STATE_HOME;
            ui_show_about();
            display_update();
            break;
    }
}
