#include "joystick.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define your joystick pins here
#define JOY_X_PIN ADC1_CHANNEL_0  // GPIO36
#define JOY_Y_PIN ADC1_CHANNEL_3  // GPIO39
#define JOY_BTN_PIN 0             // Replace with your button GPIO

// Debounce timing
static TickType_t last_action_time = 0;
static const TickType_t DEBOUNCE_DELAY = pdMS_TO_TICKS(200); // 200ms debounce

// Initialize joystick
void joystick_init(void) {
    // Configure ADC for X and Y
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(JOY_X_PIN, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(JOY_Y_PIN, ADC_ATTEN_DB_11);

    // Configure button pin
    gpio_pad_select_gpio(JOY_BTN_PIN);
    gpio_set_direction(JOY_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(JOY_BTN_PIN, GPIO_PULLUP_ONLY); // Assuming active-low button
    
    last_action_time = xTaskGetTickCount();
}

// Read joystick action
joystick_action_t joystick_get_action(void) {
    TickType_t current_time = xTaskGetTickCount();
    
    // Debounce: prevent rapid consecutive actions
    if ((current_time - last_action_time) < DEBOUNCE_DELAY) {
        return JOY_NONE;
    }
    
    int x = adc1_get_raw(JOY_X_PIN);
    int y = adc1_get_raw(JOY_Y_PIN);
    int btn = gpio_get_level(JOY_BTN_PIN);

    if (btn == 0) {  // Button pressed (active-low)
        last_action_time = current_time;
        return JOY_BTN;
    }

    if (y < JOYSTICK_THRESHOLD_LOW) {
        last_action_time = current_time;
        return JOY_UP;
    }
    if (y > JOYSTICK_THRESHOLD_HIGH) {
        last_action_time = current_time;
        return JOY_DOWN;
    }
    if (x < JOYSTICK_THRESHOLD_LOW) {
        last_action_time = current_time;
        return JOY_LEFT;
    }
    if (x > JOYSTICK_THRESHOLD_HIGH) {
        last_action_time = current_time;
        return JOY_RIGHT;
    }

    return JOY_NONE;
}
