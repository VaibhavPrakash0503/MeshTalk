/**
 * @file main.c
 * @brief MeshTalk main entry point
 *
 * Initializes hardware (I2C, joystick, display), BLE Mesh (static
 * provisioning), UI system, and application state.
 */

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

// --- Project includes ---
#include "api.h"
#include "app_state.h"       // app_state_init()
#include "display.h"         // display_init()
#include "joystick.h"        // joystick_init()
#include "mesh_init.h"       // mesh_init()
#include "message_handler.h" // message_handler_init()
#include "ui_loop.h" // ui_loop_start(), ui_loop_is_running(), ui_print_loop_stats()
#include "ui_screens.h" // ui_init()

static const char *TAG = "MAIN";

/**
 * @brief Initialize non-volatile storage (needed for BLE Mesh)
 */
static void nvs_init(void) {
  ESP_LOGI(TAG, "Initializing NVS...");
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "NVS partition truncated, erasing...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_LOGI(TAG, "NVS initialized successfully");
}

/**
 * @brief Hardware initialization
 */
static esp_err_t hardware_init(void) {
  ESP_LOGI(TAG, "Initializing hardware...");

  display_init();
  ESP_LOGI(TAG, "Display initialized");

  joystick_init();
  ESP_LOGI(TAG, "Joystick initialized");

  return ESP_OK;
}

/**
 * @brief Application layer initialization
 */
static esp_err_t app_layer_init(void) {
  ESP_LOGI(TAG, "Initializing application layer...");

  app_state_init();
  ESP_LOGI(TAG, "App state initialized");

  message_handler_init();
  ESP_LOGI(TAG, "Message handler ready");

  message_handler_register_app_cb(api_on_message_received);
  ESP_LOGI(TAG, "API callback registered with message handler");

  return ESP_OK;
}

/**
 * @brief System monitoring task
 */
static void system_monitor_task(void *pvParameters) {
  const TickType_t delay = pdMS_TO_TICKS(30000); // 30s
  while (1) {
    ESP_LOGI(TAG, "System OK - Free heap: %d, Min free: %d",
             esp_get_free_heap_size(), esp_get_minimum_free_heap_size());

    if (ui_loop_is_running()) {
      ESP_LOGI(TAG, "UI loop running");
      ui_print_loop_stats();
    } else {
      ESP_LOGW(TAG, "UI loop not running!");
    }

    vTaskDelay(delay);
  }
}

/**
 * @brief Main entry point
 */
void app_main(void) {
  ESP_LOGI(TAG, "===== MeshTalk Boot =====");

  // Step 1: NVS
  nvs_init();

  // Step 2: Hardware
  if (hardware_init() != ESP_OK) {
    ESP_LOGE(TAG, "Hardware init failed!");
    return;
  }

  // Step 3: App layer
  if (app_layer_init() != ESP_OK) {
    ESP_LOGE(TAG, "App layer init failed!");
    return;
  }

  // Step 4: UI system (calls api_init internally)
  ui_init();
  ui_loop_start();
  ESP_LOGI(TAG, "UI system initialized");

  // Step 5: Mesh networking
  mesh_init();
  ESP_LOGI(TAG, "Mesh network initialized");

  check_provisioning_status();

  // Step 6: System monitor
  xTaskCreate(system_monitor_task, "sys_monitor", 2048, NULL, 1, NULL);

  ESP_LOGI(TAG, "===== MeshTalk Ready =====");
}
