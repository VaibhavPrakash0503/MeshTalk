#include "ui_loop.h"
#include "ui_screens.h"
#include "app_state.h"
#include "joystick.h"

// Main UI loop function - call this repeatedly in your main loop
void ui_main_loop(void) {
    screen_t current_screen = app_state_get_screen();
    
    switch (current_screen) {
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
            ui_render_screen(SCREEN_HOME);
            break;
    }
}
