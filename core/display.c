#include "display.h"
#include "ssd1306.h"
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000

static SSD1306_t dev;

static void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void display_init(void)
{
    i2c_master_init();
    ssd1306_init(&dev, I2C_MASTER_NUM, 128, 64);
    ssd1306_clear_screen(&dev, false);
}

void display_clear(void)
{
    ssd1306_clear_screen(&dev, false);
}

void display_draw_text(int x, int y, const char *text)
{
    ssd1306_display_text(&dev, y, text, strlen(text), false);
}

void display_update(void)
{
    // If needed for buffered drawing (depends on SSD1306 lib version)
}

