#include "display.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include <string.h>

static const char *TAG = "DISPLAY";
static SSD1306_t dev;
static bool display_initialized = false;
static display_mode_t current_mode = DISPLAY_MODE_MENU;

// ✅ Optimized memory allocation
static char current_lines[MAX_DISPLAY_LINES][MAX_CHARS_PER_LINE + 1];
static bool line_has_cursor[MAX_DISPLAY_LINES] = {false};

// ✅ Concurrency safety
static SemaphoreHandle_t display_mutex = NULL;

// ✅ Thread-safe SSD1306 wrapper functions
static esp_err_t safe_ssd1306_display_text(int page, const char *text,
                                           size_t len, bool invert) {
  esp_err_t ret = ESP_FAIL;
  if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    ssd1306_display_text(&dev, page, text, len, invert);
    xSemaphoreGive(display_mutex);
  } else {
    ESP_LOGW(TAG, "Display mutex timeout");
  }
  return ret;
}

static esp_err_t safe_ssd1306_clear_screen(bool invert) {
  esp_err_t ret = ESP_FAIL;
  if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    ssd1306_clear_screen(&dev, invert);
    xSemaphoreGive(display_mutex);
  } else {
    ESP_LOGW(TAG, "Display mutex timeout on clear");
  }
  return ret;
}

// ✅ True large font simulation (double-height)
static void draw_large_text(int start_page, const char *text, bool invert) {
  // Draw on two consecutive pages for double height effect
  safe_ssd1306_display_text(start_page, text, strlen(text), invert);
  safe_ssd1306_display_text(start_page + 1, text, strlen(text), invert);
}

// I2C initialization (same as before)
static esp_err_t i2c_master_init_display(void) {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
  if (err != ESP_OK)
    return err;

  err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
  if (err != ESP_OK)
    return err;

  ESP_LOGI(TAG, "✅ I2C initialized: SDA=%d, SCL=%d", I2C_MASTER_SDA_IO,
           I2C_MASTER_SCL_IO);
  return ESP_OK;
}

void display_init(void) {
  ESP_LOGI(TAG, "Initializing SSD1306 display for MeshTalk...");

  // ✅ Create display mutex for thread safety
  display_mutex = xSemaphoreCreateMutex();
  if (!display_mutex) {
    ESP_LOGE(TAG, "Failed to create display mutex");
    return;
  }

  if (i2c_master_init_display() != ESP_OK) {
    ESP_LOGE(TAG, "❌ Failed to initialize I2C bus");
    return;
  }

  i2c_master_init(&dev, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, -1);
  ESP_LOGI(TAG, "✅ SSD1306 I2C initialized");

  ssd1306_init(&dev, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  ESP_LOGI(TAG, "✅ SSD1306 display initialized");

  // Clear and initialize tracking
  safe_ssd1306_clear_screen(false);
  memset(current_lines, 0, sizeof(current_lines));
  memset(line_has_cursor, false, sizeof(line_has_cursor));

  display_initialized = true;
  ESP_LOGI(TAG, "✅ SSD1306 display initialized with thread safety");
}

void display_clear(void) {
  if (!display_initialized)
    return;

  safe_ssd1306_clear_screen(false);
  memset(current_lines, 0, sizeof(current_lines));
  memset(line_has_cursor, false, sizeof(line_has_cursor));
}

void display_set_mode(display_mode_t mode) {
  current_mode = mode;
  ESP_LOGI(TAG, "Display mode: %s",
           (mode == DISPLAY_MODE_MENU)   ? "MENU"
           : (mode == DISPLAY_MODE_LIST) ? "LIST"
                                         : "CHAT");
}

display_mode_t display_get_mode(void) { return current_mode; }

// ✅ TEXT ALIGNMENT HELPERS
void display_center_text(int line, const char *text, bool large_font) {
  if (!display_initialized || line < 0 || line >= MAX_DISPLAY_LINES)
    return;

  int text_len = strlen(text);
  if (text_len > MAX_CHARS_PER_LINE)
    text_len = MAX_CHARS_PER_LINE;

  int padding = (MAX_CHARS_PER_LINE - text_len) / 2;
  char centered_text[MAX_CHARS_PER_LINE + 1];

  // Create centered string
  memset(centered_text, ' ', padding);
  memcpy(centered_text + padding, text, text_len);
  centered_text[padding + text_len] = '\0';

  // Draw with appropriate font
  if (large_font && line <= 3) {
    draw_large_text(line * 2, centered_text, false);
  } else {
    safe_ssd1306_display_text(line, centered_text, strlen(centered_text),
                              false);
  }

  // Update tracking
  strncpy(current_lines[line], centered_text, MAX_CHARS_PER_LINE);
}

void display_left_text(int line, const char *text, bool large_font) {
  if (!display_initialized || line < 0 || line >= MAX_DISPLAY_LINES)
    return;

  // Draw with appropriate font
  if (large_font && line <= 3) {
    draw_large_text(line * 2, text, false);
  } else {
    safe_ssd1306_display_text(line, text, strlen(text), false);
  }

  // Update tracking
  strncpy(current_lines[line], text, MAX_CHARS_PER_LINE);
}

// ✅ IMPROVED MENU FUNCTIONS (True large font)
void display_menu_line_large(int line, const char *text, bool has_cursor) {
  if (!display_initialized || line < 0 || line > 3)
    return;

  char display_text[MAX_CHARS_PER_LINE + 1];

  // Add cursor prefix if needed
  if (has_cursor) {
    snprintf(display_text, sizeof(display_text), "> %s", text);
    draw_large_text(line * 2, display_text, true); // Inverted for selection
  } else {
    snprintf(display_text, sizeof(display_text), "  %s", text);
    draw_large_text(line * 2, display_text, false);
  }

  // Update tracking
  strncpy(current_lines[line], display_text, MAX_CHARS_PER_LINE);
  line_has_cursor[line] = has_cursor;

  ESP_LOGD(TAG, "Large menu line %d: %s %s", line, text,
           has_cursor ? "[CURSOR]" : "");
}

void display_show_menu_screen(const char *title, const char *menu_items[],
                              int item_count, int cursor_pos) {
  if (!display_initialized)
    return;

  display_clear();

  // Line 0: Centered title (large font, no cursor)
  display_center_text(0, title, true);

  // Lines 1-3: Menu items (large font with cursor)
  int max_items = (item_count > 3) ? 3 : item_count;
  for (int i = 0; i < max_items; i++) {
    bool has_cursor = (i == cursor_pos);
    display_menu_line_large(i + 1, menu_items[i], has_cursor);
  }

  ESP_LOGD(TAG, "Menu screen: %s, %d items, cursor at %d", title, max_items,
           cursor_pos);
}

// ✅ LIST FUNCTIONS (Small font, optimized)
void display_list_line(int line, const char *text, bool has_cursor) {
  if (!display_initialized || line < 0 || line >= MAX_DISPLAY_LINES)
    return;

  char display_text[MAX_CHARS_PER_LINE + 1];

  // Add cursor prefix if needed
  if (has_cursor) {
    snprintf(display_text, sizeof(display_text), "> %s", text);
  } else {
    snprintf(display_text, sizeof(display_text), "  %s", text);
  }

  safe_ssd1306_display_text(line, display_text, strlen(display_text), false);

  // Update tracking
  strncpy(current_lines[line], display_text, MAX_CHARS_PER_LINE);
  line_has_cursor[line] = has_cursor;
}

void display_show_list_screen(const char *title, const char *list_items[],
                              int item_count, int cursor_pos,
                              int scroll_offset) {
  if (!display_initialized)
    return;

  display_clear();

  // Line 0: Centered title
  display_center_text(0, title, false);

  // Lines 1-7: List items (7 visible items)
  int visible_items = 7;
  for (int i = 0; i < visible_items; i++) {
    int item_index = scroll_offset + i;
    if (item_index < item_count) {
      bool has_cursor = (item_index == cursor_pos);
      display_list_line(i + 1, list_items[item_index], has_cursor);
    } else {
      display_list_line(i + 1, "", false); // Empty line
    }
  }

  ESP_LOGD(TAG, "List screen: %s, %d items, cursor at %d, scroll %d", title,
           item_count, cursor_pos, scroll_offset);
}

void display_show_chat_screen(const char *contact_name, const char *messages[],
                              int msg_count, int scroll_offset) {
  if (!display_initialized)
    return;

  display_clear();

  // Line 0: Centered contact name
  display_center_text(0, contact_name, false);

  // Lines 1-7: Chat messages (7 visible messages)
  int visible_messages = 7;
  for (int i = 0; i < visible_messages; i++) {
    int msg_index = scroll_offset + i;
    if (msg_index < msg_count && messages[msg_index]) {
      display_list_line(i + 1, messages[msg_index], false);
    } else {
      display_list_line(i + 1, "", false); // Empty line
    }
  }

  ESP_LOGD(TAG, "Chat screen: %s, %d messages, scroll %d", contact_name,
           msg_count, scroll_offset);
}

// ✅ THREAD-SAFE CURSOR MOVEMENT
void display_move_cursor(int old_line, int new_line) {
  if (!display_initialized || old_line == new_line)
    return;

  // Remove cursor from old line
  if (old_line >= 0 && old_line < MAX_DISPLAY_LINES &&
      line_has_cursor[old_line]) {
    char text_only[MAX_CHARS_PER_LINE + 1];
    // Extract text without cursor prefix
    const char *src = current_lines[old_line];
    if (src[0] == '>' && src[1] == ' ') {
      strncpy(text_only, src + 2, MAX_CHARS_PER_LINE);
    } else {
      strncpy(text_only, src, MAX_CHARS_PER_LINE);
    }

    if (current_mode == DISPLAY_MODE_MENU && old_line <= 3) {
      display_menu_line_large(old_line, text_only, false);
    } else {
      display_list_line(old_line, text_only, false);
    }
  }

  // Add cursor to new line
  if (new_line >= 0 && new_line < MAX_DISPLAY_LINES) {
    char text_only[MAX_CHARS_PER_LINE + 1];
    // Extract text without cursor prefix
    const char *src = current_lines[new_line];
    if (src[0] == ' ' && src[1] == ' ') {
      strncpy(text_only, src + 2, MAX_CHARS_PER_LINE);
    } else {
      strncpy(text_only, src, MAX_CHARS_PER_LINE);
    }

    if (current_mode == DISPLAY_MODE_MENU && new_line <= 3) {
      display_menu_line_large(new_line, text_only, true);
    } else {
      display_list_line(new_line, text_only, true);
    }
  }
}

void display_update(void) {
  if (!display_initialized)
    return;
  // Most SSD1306 libraries update immediately
}

bool display_is_ready(void) { return display_initialized; }
