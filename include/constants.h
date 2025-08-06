// This file contains the constants used across the project

#pragma once

#define MAX_PLAYLOAD_SIZE 255
#define MAX_MESSAGE_LEN 100

#define HEARTBEAT_TIMEOUT_MS  10000   // 10 seconds of silence = offline

#define SYMBOL_ONLINE         "●"
#define SYMBOL_OFFLINE        "○"
#define SYMBOL_NEW_MESSAGE    "*"

// Misc
#define MAX_USERS             10      // Max number of known users in the chat
#define USERNAME_MAX_LEN      10      // Limit for display names
