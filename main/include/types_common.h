#pragma once

// Application screens
typedef enum {
  SCREEN_HOME,            // Main menu (Chat, Broadcast)
  SCREEN_CHAT,            // Chat list screen
  SCREEN_INDIVIDUAL_CHAT, // Individual chat conversation
  SCREEN_SEND_MESSAGE,    // Send message options
  SCREEN_BROADCAST,       // Broadcasting status - ADD COMMA HERE
  SCREEN_ABOUT            // About screen
} screen_t;
// Generic status
typedef enum { STATUS_OK, STATUS_ERROR, STATUS_BUSY } status_t;
