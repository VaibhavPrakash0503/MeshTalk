#include "display.h"
#include "ssd1306.h"
#include <string.h>

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

static SSD1306_t dev;

void display_init(void) {
  i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

  ssd1306_init(&dev, 128, 64);

  ssd1306_clear_screen(&dev, false);
}

void display_clear(void) { ssd1306_clear_screen(&dev, false); }

void display_draw_text(int y, const char *text) {
  ssd1306_display_text(&dev, y, text, strlen(text), false);
}

void display_update(void) {
  // If needed for buffered drawing (depends on SSD1306 lib version)
}
