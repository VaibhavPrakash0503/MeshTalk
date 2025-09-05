#pragma once

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "joystick.h"
#include "ui_screens.h"

// UI loop configuration
#define UI_TASK_NAME "ui_task"
#define UI_TASK_STACK_SIZE 4096
#define UI_TASK_PRIORITY 5
#define UI_UPDATE_INTERVAL_MS 50 // 20 FPS
#define UI_INPUT_DEBOUNCE_MS 20  // Input debounce time

// Task handle for external control
extern TaskHandle_t ui_task_handle;

// UI loop control functions
void ui_loop_start(void);
void ui_loop_stop(void);
void ui_loop_pause(void);
void ui_loop_resume(void);
bool ui_loop_is_running(void);

// Core loop functions
void ui_process_input(void);
void ui_process_timeouts(void);
void ui_update_display(void);

// Debug functions
void ui_print_loop_stats(void);
