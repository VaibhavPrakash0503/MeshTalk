#pragma once
#include "constants.h"
#include "message_struct.h"
#include "types_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Structure representing a single chat message
 */
typedef struct {
  MeshMessage msg;
} chat_entry_t;

/**
 * @brief Per-user chat log (fixed length buffer, circular)
 */
typedef struct {
  chat_entry_t entries[CHAT_LOG_SIZE]; // last N messages
  int head;                            // next insert position
  int count;                           // number of valid messages
} chat_log_t;

/**
 * @brief Global application state
 */
typedef struct {
  screen_t current_screen;
  int selected_user;
  bool new_message_flag;
  chat_log_t chat_logs[MAX_USERS]; // one log per user
} app_state_t;

// Global instance (defined in app_state.c)
extern app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void);

/**
 * @brief Add a message to the chat log for a user
 */
void app_state_add_message(const MeshMessage *msg);

/**
 * @brief Retrieve last N messages for a user
 * @param user_id The user whose chat log to query
 * @param out_entries Array to fill with results
 * @param max_entries Capacity of out_entries
 * @return Number of messages returned
 */
int app_state_get_messages(int user_id, chat_entry_t *out_entries,
                           int max_entries);
