#pragma once
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "stdbool.h"

// I2C Configuration
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define SSD1306_I2C_ADDRESS 0x3C

// Display dimensions and optimization
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define CHAR_WIDTH 6 // 5px font + 1px spacing
#define MAX_CHARS_PER_LINE (DISPLAY_WIDTH / CHAR_WIDTH) // ~21 chars
#define MAX_DISPLAY_LINES 8

// Display modes
typedef enum {
  DISPLAY_MODE_MENU,
  DISPLAY_MODE_LIST,
  DISPLAY_MODE_CHAT
} display_mode_t;

// Function declarations
void display_init(void);
void display_clear(void);
void display_set_mode(display_mode_t mode);
display_mode_t display_get_mode(void);

// Menu functions with true large font support
void display_menu_line_large(int line, const char *text, bool has_cursor);
void display_show_menu_screen(const char *title, const char *menu_items[],
                              int item_count, int cursor_pos);

// List functions (optimized)
void display_list_line(int line, const char *text, bool has_cursor);
void display_show_list_screen(const char *title, const char *list_items[],
                              int item_count, int cursor_pos,
                              int scroll_offset);

// Chat functions
void display_show_chat_screen(const char *contact_name, const char *messages[],
                              int msg_count, int scroll_offset);

// Text alignment helpers
void display_center_text(int line, const char *text, bool large_font);
void display_left_text(int line, const char *text, bool large_font);

// Smooth cursor operations (thread-safe)
void display_move_cursor(int old_line, int new_line);

// Generic functions
void display_update(void);
bool display_is_ready(void);
