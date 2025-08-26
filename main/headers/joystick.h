
#pragma once

// Joystick movement thresholds
#define JOYSTICK_THRESHOLD_LOW   1000
#define JOYSTICK_THRESHOLD_HIGH  3000

typedef enum {
    JOY_NONE,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_BTN
} joystick_action_t;

void joystick_init(void);
joystick_action_t joystick_get_action(void);
