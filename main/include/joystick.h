#pragma once
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

// Storage namespace
#define JOYSTICK_NVS_NAMESPACE "joystick"

// Calibrated thresholds
extern int32_t joystick_center_x, joystick_center_y;
extern int32_t joystick_threshold_x_low, joystick_threshold_x_high;
extern int32_t joystick_threshold_y_low, joystick_threshold_y_high;

typedef enum {
  JOY_NONE,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_BTN
} joystick_action_t;

void joystick_init(void);
void joystick_force_calibrate(void); // Manual calibration
joystick_action_t joystick_get_action(void);
bool joystick_is_calibrated(void);
