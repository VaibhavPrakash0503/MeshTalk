#include "joystick.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define your joystick pins here
#define JOY_X_PIN ADC1_CHANNEL_0  // GPIO36
#define JOY_Y_PIN ADC1_CHANNEL_3  // GPIO39
#define JOY_BTN_PIN 0             // Replace with your button GPIO

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
}

// Read joystick action
joystick_action_t joystick_get_action(void) {
    int x = adc1_get_raw(JOY_X_PIN);
    int y = adc1_get_raw(JOY_Y_PIN);
    int btn = gpio_get_level(JOY_BTN_PIN);

    if (btn == 0) return JOY_BTN;  // Button pressed (active-low)

    if (y < JOYSTICK_THRESHOLD_LOW) return JOY_UP;
    if (y > JOYSTICK_THRESHOLD_HIGH) return JOY_DOWN;
    if (x < JOYSTICK_THRESHOLD_LOW) return JOY_LEFT;
    if (x > JOYSTICK_THRESHOLD_HIGH) return JOY_RIGHT;

    return JOY_NONE;
}
