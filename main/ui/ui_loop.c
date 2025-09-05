#include "ui_loop.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "UI_LOOPS";

// Task handles and control
TaskHandle_t ui_task_handle = NULL;
static SemaphoreHandle_t ui_loop_mutex = NULL;

// Loop state
static bool ui_running = false;
static bool ui_paused = false;

// Statistics
static uint32_t frame_count = 0;
static int64_t last_update_time = 0;

// Input debouncing
static int64_t last_input_time = 0;

// Main UI loop task
void ui_main_loop(void *pvParameters) {
  ESP_LOGI(TAG, "UI main loop started");

  TickType_t last_wake_time = xTaskGetTickCount();

  while (ui_running) {
    // Check if paused
    if (xSemaphoreTake(ui_loop_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      if (!ui_paused) {
        // ✅ Process joystick input with debouncing
        ui_process_input();

        // ✅ Handle timeouts (broadcast timeout, etc.)
        ui_process_timeouts();

        // ✅ Update display if needed
        ui_update_display();

        // Update statistics
        frame_count++;
        last_update_time = esp_timer_get_time();
      }
      xSemaphoreGive(ui_loop_mutex);
    }

    // ✅ Maintain consistent frame rate (20 FPS)
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(UI_UPDATE_INTERVAL_MS));
  }

  ESP_LOGI(TAG, "UI main loop ended");
  vTaskDelete(NULL);
}

void ui_loop_start(void) {
  if (ui_task_handle) {
    ESP_LOGW(TAG, "UI loop already running");
    return;
  }

  ESP_LOGI(TAG, "Starting UI loop...");

  // Create mutex for thread safety
  ui_loop_mutex = xSemaphoreCreateMutex();
  if (!ui_loop_mutex) {
    ESP_LOGE(TAG, "Failed to create UI loop mutex");
    return;
  }

  // Initialize UI system
  ui_init();

  // Create UI task
  ui_running = true;
  ui_paused = false;

  BaseType_t result =
      xTaskCreate(ui_main_loop, UI_TASK_NAME, UI_TASK_STACK_SIZE, NULL,
                  UI_TASK_PRIORITY, &ui_task_handle);

  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create UI task");
    ui_running = false;
    vSemaphoreDelete(ui_loop_mutex);
    ui_loop_mutex = NULL;
    return;
  }

  ESP_LOGI(TAG, "✅ UI loop started successfully");
}

void ui_loop_stop(void) {
  if (!ui_task_handle) {
    ESP_LOGW(TAG, "UI loop not running");
    return;
  }

  ESP_LOGI(TAG, "Stopping UI loop...");

  // Stop the loop
  ui_running = false;

  // Wait for task to finish
  vTaskDelay(pdMS_TO_TICKS(100));

  // Clean up resources
  if (ui_loop_mutex) {
    vSemaphoreDelete(ui_loop_mutex);
    ui_loop_mutex = NULL;
  }

  ui_task_handle = NULL;
  ESP_LOGI(TAG, "UI loop stopped");
}

void ui_loop_pause(void) {
  if (ui_loop_mutex &&
      xSemaphoreTake(ui_loop_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    ui_paused = true;
    xSemaphoreGive(ui_loop_mutex);
    ESP_LOGI(TAG, "UI loop paused");
  }
}

void ui_loop_resume(void) {
  if (ui_loop_mutex &&
      xSemaphoreTake(ui_loop_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    ui_paused = false;
    xSemaphoreGive(ui_loop_mutex);
    ESP_LOGI(TAG, "UI loop resumed");
  }
}

bool ui_loop_is_running(void) { return ui_running && !ui_paused; }

// ✅ INPUT PROCESSING with debouncing
void ui_process_input(void) {
  int64_t current_time = esp_timer_get_time();

  // Debounce input to prevent multiple activations
  if (current_time - last_input_time < UI_INPUT_DEBOUNCE_MS * 1000) {
    return;
  }

  joystick_action_t action = joystick_get_action();

  if (action != JOY_NONE) {
    ESP_LOGD(TAG, "Joystick action: %d", action);
    ui_handle_joystick(action);
    last_input_time = current_time;
  }
}

// ✅ TIMEOUT PROCESSING
void ui_process_timeouts(void) {
  // Check broadcast timeout and other UI timeouts
  ui_check_broadcast_timeout();

  // Add other timeout checks here if needed
  // e.g., screen saver timeout, auto-refresh, etc.
}

// ✅ DISPLAY UPDATE PROCESSING
void ui_update_display(void) {
  if (ui_needs_update()) {
    ui_update();
  }
}

// ✅ DEBUG FUNCTIONS
void ui_print_loop_stats(void) {
  ESP_LOGI(TAG, "UI Loop Statistics:");
  ESP_LOGI(TAG, "  Running: %s", ui_running ? "YES" : "NO");
  ESP_LOGI(TAG, "  Paused: %s", ui_paused ? "YES" : "NO");
  ESP_LOGI(TAG, "  Frame count: %lu", frame_count);
  ESP_LOGI(TAG, "  Last update: %lld µs ago",
           esp_timer_get_time() - last_update_time);
  ESP_LOGI(TAG, "  Task handle: %p", ui_task_handle);
}
