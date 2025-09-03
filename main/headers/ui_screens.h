
#pragma once
#include "types_common.h"

extern const char *predefined_msgs[];
extern const int num_predefined_msgs;

void ui_init(void);
void ui_render_screen(screen_t screen);
void ui_show_home(void);
void ui_show_chat(void);
void ui_show_individual_chat(int user_id);
void ui_show_send_message_menu(int user_id);
void ui_show_about(void);
