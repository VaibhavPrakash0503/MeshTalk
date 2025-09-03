#pragma once

#define HEARTBEAT_TIMEOUT_MS 10000 // 10 seconds of silence = offline

#define SYMBOL_ONLINE "●"
#define SYMBOL_OFFLINE "○"
#define SYMBOL_NEW_MESSAGE "*"

// Misc
#define MAX_USERS 3 // Max number of known users in the chat
#define USERNAME_MAX_LEN 5
#define CHAT_LOG_SIZE 15 // Limit for display names
