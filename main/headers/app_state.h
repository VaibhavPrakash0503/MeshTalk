
#pragma once
#include "types_common.h"

// Structure for global app state
typedef struct {
    screen_t current_screen;
    int selected_user;
    bool new_message_flag;
} app_state_t;

// Global state instance (defined in app_state.c)
extern app_state_t g_app_state;

// State management functions
void app_state_init(void);
