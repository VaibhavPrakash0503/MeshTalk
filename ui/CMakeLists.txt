idf_component_register(
    SRCS
        "ui_screen.c"
        "joystick.c"
    INCLUDE_DIRS
        "../include"
        "../components/ssd1306"  # Only if SSD1306 driver is here
)

