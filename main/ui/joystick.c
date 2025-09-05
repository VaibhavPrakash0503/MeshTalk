#include "joystick.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

static const char *TAG = "JOYSTICK";

static adc_oneshot_unit_handle_t adc1_handle;

// Pin definitions
#define JOY_X_PIN ADC_CHANNEL_6 // GPIO34 (D34)
#define JOY_Y_PIN ADC_CHANNEL_7 // GPIO35 (D35)
#define JOY_BTN_PIN GPIO_NUM_32 // GPIO32 (D32)

// Smoothing filter
#define FILTER_SAMPLES 5
static int x_history[FILTER_SAMPLES] = {0};
static int y_history[FILTER_SAMPLES] = {0};
static int filter_index = 0;

// Calibrated values
int32_t joystick_center_x = 2048;
int32_t joystick_center_y = 2048;
int32_t joystick_threshold_x_low = 1500;
int32_t joystick_threshold_x_high = 2500;
int32_t joystick_threshold_y_low = 1500;
int32_t joystick_threshold_y_high = 2500;

// Debounce
static volatile TickType_t last_action_time = 0;
static const TickType_t DEBOUNCE_DELAY = pdMS_TO_TICKS(200);

// ✅ Save calibration to NVS
static void save_calibration(void) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open(JOYSTICK_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err == ESP_OK) {
    nvs_set_i32(nvs_handle, "center_x", joystick_center_x);
    nvs_set_i32(nvs_handle, "center_y", joystick_center_y);
    nvs_set_i32(nvs_handle, "low_x", joystick_threshold_x_low);
    nvs_set_i32(nvs_handle, "high_x", joystick_threshold_x_high);
    nvs_set_i32(nvs_handle, "low_y", joystick_threshold_y_low);
    nvs_set_i32(nvs_handle, "high_y", joystick_threshold_y_high);
    nvs_set_u8(nvs_handle, "calibrated", 1); // Calibration flag
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "💾 Calibration saved to NVS");
  }
}

// ✅ Load calibration from NVS
static bool load_calibration(void) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open(JOYSTICK_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
  if (err != ESP_OK)
    return false;

  uint8_t calibrated = 0;
  if (nvs_get_u8(nvs_handle, "calibrated", &calibrated) != ESP_OK ||
      calibrated != 1) {
    nvs_close(nvs_handle);
    return false;
  }

  // Load all calibration values
  bool success = true;
  success &=
      (nvs_get_i32(nvs_handle, "center_x", &joystick_center_x) == ESP_OK);
  success &=
      (nvs_get_i32(nvs_handle, "center_y", &joystick_center_y) == ESP_OK);
  success &=
      (nvs_get_i32(nvs_handle, "low_x", &joystick_threshold_x_low) == ESP_OK);
  success &=
      (nvs_get_i32(nvs_handle, "high_x", &joystick_threshold_x_high) == ESP_OK);
  success &=
      (nvs_get_i32(nvs_handle, "low_y", &joystick_threshold_y_low) == ESP_OK);
  success &=
      (nvs_get_i32(nvs_handle, "high_y", &joystick_threshold_y_high) == ESP_OK);

  nvs_close(nvs_handle);

  if (success) {
    ESP_LOGI(TAG, "📂 Loaded calibration from NVS");
    ESP_LOGI(TAG, "   Center: X=%d, Y=%d", joystick_center_x,
             joystick_center_y);
  }

  return success;
}

// ✅ Smoothing filter
static int smooth_adc_reading(int new_value, int *history) {
  history[filter_index] = new_value;

  int sum = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    sum += history[i];
  }

  return sum / FILTER_SAMPLES;
}

// ✅ Check if manual calibration requested (long-press button during init)
static bool calibration_requested(void) {
  ESP_LOGI(TAG, "🔘 Hold button for 3 seconds to recalibrate...");

  for (int i = 0; i < 30; i++) {            // 3 seconds = 30 * 100ms
    if (gpio_get_level(JOY_BTN_PIN) != 0) { // Button released
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  ESP_LOGI(TAG, "🎯 Manual calibration requested!");
  return true;
}

void joystick_init(void) {
  ESP_LOGI(TAG, "Initializing MeshTalk joystick...");

  // Initialize ADC1 with new driver
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  // Configure ADC1 channels
  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_12,
      .atten = ADC_ATTEN_DB_12, // Use ADC_ATTEN_DB_12 instead of deprecated
                                // ADC_ATTEN_DB_11
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOY_X_PIN, &config));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOY_Y_PIN, &config));

  // Configure button (same as before)
  ESP_ERROR_CHECK(gpio_reset_pin(JOY_BTN_PIN));
  ESP_ERROR_CHECK(gpio_set_direction(JOY_BTN_PIN, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_pull_mode(JOY_BTN_PIN, GPIO_PULLUP_ONLY));

  // Initialize filter with stability delay
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    int raw_x, raw_y;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_X_PIN, &raw_x));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_Y_PIN, &raw_y));
    x_history[i] = raw_x;
    y_history[i] = raw_y;
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // Rest of initialization remains the same...
  last_action_time = xTaskGetTickCount();
  bool need_calibration = !load_calibration();
  if (!need_calibration) {
    need_calibration = calibration_requested();
  }
  if (need_calibration) {
    joystick_force_calibrate();
    save_calibration();
  }
  ESP_LOGI(TAG, "✅ MeshTalk joystick ready!");
}

// ✅ User-friendly calibration with clear instructions

void joystick_force_calibrate(void) {
  ESP_LOGI(TAG, "🎯 === JOYSTICK CALIBRATION MODE ===");
  ESP_LOGI(TAG, "");
  ESP_LOGI(TAG, "Step 1: Keep joystick CENTERED...");
  // Countdown for centering
  for (int i = 3; i > 0; i--) {
    ESP_LOGI(TAG, "Centering in %d seconds...", i);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // Sample center position - use int32_t for consistency
  int32_t sum_x = 0, sum_y = 0;
  const int center_samples = 50;
  int raw_x, raw_y;

  for (int i = 0; i < center_samples; i++) {
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_X_PIN, &raw_x));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_Y_PIN, &raw_y));
    sum_x += raw_x;
    sum_y += raw_y;
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  joystick_center_x = sum_x / center_samples;
  joystick_center_y = sum_y / center_samples;

  ESP_LOGI(TAG, "✅ Center recorded: X=%d, Y=%d", joystick_center_x,
           joystick_center_y);
  ESP_LOGI(TAG, "");
  ESP_LOGI(TAG, "Step 2: Move joystick in ALL directions...");
  ESP_LOGI(TAG, "UP, DOWN, LEFT, RIGHT - keep moving for 5 seconds!");

  // Sample range
  int min_x = 4095, max_x = 0;
  int min_y = 4095, max_y = 0;
  const int range_samples = 250; // 5 seconds
  int x, y;

  for (int i = 0; i < range_samples; i++) {
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_X_PIN, &x));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOY_Y_PIN, &y));

    min_x = (x < min_x) ? x : min_x;
    max_x = (x > max_x) ? x : max_x;
    min_y = (y < min_y) ? y : min_y;
    max_y = (y > max_y) ? y : max_y;

    // Progress indicator every second
    if (i % 50 == 0) {
      ESP_LOGI(TAG, "Keep moving... (%d/5 seconds)", i / 50 + 1);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  // Calculate dynamic thresholds
  int x_range = (max_x - min_x) / 2;
  int y_range = (max_y - min_y) / 2;

  joystick_threshold_x_low = joystick_center_x - (x_range * 30 / 100);
  joystick_threshold_x_high = joystick_center_x + (x_range * 30 / 100);
  joystick_threshold_y_low = joystick_center_y - (y_range * 30 / 100);
  joystick_threshold_y_high = joystick_center_y + (y_range * 30 / 100);

  ESP_LOGI(TAG, "");
  ESP_LOGI(TAG, "🎉 === CALIBRATION COMPLETE ===");
  ESP_LOGI(TAG, "Center: X=%d, Y=%d", joystick_center_x, joystick_center_y);
  ESP_LOGI(TAG, "X Range: %d-%d (thresholds: %d/%d)", min_x, max_x,
           joystick_threshold_x_low, joystick_threshold_x_high);
  ESP_LOGI(TAG, "Y Range: %d-%d (thresholds: %d/%d)", min_y, max_y,
           joystick_threshold_y_low, joystick_threshold_y_high);
  ESP_LOGI(TAG, "🕹️ Test your movements now!");
}
joystick_action_t joystick_get_action(void) {
  TickType_t current_time = xTaskGetTickCount();

  if ((current_time - last_action_time) < DEBOUNCE_DELAY) {
    return JOY_NONE;
  }

  // Read and smooth ADC values
  int raw_x, raw_y;
  esp_err_t ret_x = adc_oneshot_read(adc1_handle, JOY_X_PIN, &raw_x);
  esp_err_t ret_y = adc_oneshot_read(adc1_handle, JOY_Y_PIN, &raw_y);

  if (ret_x != ESP_OK || ret_y != ESP_OK) {
    ESP_LOGW(TAG, "ADC read error: X=%s, Y=%s", esp_err_to_name(ret_x),
             esp_err_to_name(ret_y));
    return JOY_NONE;
  }
  int x = smooth_adc_reading(raw_x, x_history);
  int y = smooth_adc_reading(raw_y, y_history);
  filter_index = (filter_index + 1) % FILTER_SAMPLES;

  int btn = gpio_get_level(JOY_BTN_PIN);

  joystick_action_t action = JOY_NONE;

  if (btn == 0) {
    action = JOY_BTN;
  } else if (y < joystick_threshold_y_low) {
    action = JOY_UP;
  } else if (y > joystick_threshold_y_high) {
    action = JOY_DOWN;
  } else if (x < joystick_threshold_x_low) {
    action = JOY_LEFT;
  } else if (x > joystick_threshold_x_high) {
    action = JOY_RIGHT;
  }

  if (action != JOY_NONE) {
    last_action_time = current_time;
  }

  return action;
}

bool joystick_is_calibrated(void) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open(JOYSTICK_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
  if (err != ESP_OK)
    return false;

  uint8_t calibrated = 0;
  bool result = (nvs_get_u8(nvs_handle, "calibrated", &calibrated) == ESP_OK &&
                 calibrated == 1);
  nvs_close(nvs_handle);
  return result;
}
