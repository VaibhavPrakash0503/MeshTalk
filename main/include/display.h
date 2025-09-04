#pragma once

// Initialize the display hardware
void display_init(void);

// Clear the display
void display_clear(void);

// Draw a string at coordinates
void display_draw_text(int y, const char *text);

// Refresh the display
void display_update(void);
