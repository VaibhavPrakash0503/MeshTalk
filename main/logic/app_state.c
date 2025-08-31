#include "app_state.h"
#include <string.h>

// Global state instance
app_state_t g_app_state;

/**
 * @brief Initialize global app state
 */
void app_state_init(void) {
  g_app_state.current_screen = SCREEN_HOME;
  g_app_state.selected_user = -1;
  g_app_state.new_message_flag = false;

  // Clear logs
  for (int i = 0; i < MAX_USERS; i++) {
    g_app_state.chat_logs[i].head = 0;
    g_app_state.chat_logs[i].count = 0;
    memset(g_app_state.chat_logs[i].entries, 0,
           sizeof(g_app_state.chat_logs[i].entries));
  }
}

/**
 * @brief Add a message to the user’s chat log
 */
void app_state_add_message(const MeshMessage *msg) {
  if (!msg || msg->receiver_id >= MAX_USERS) {
    return;
  }

  int user_id = msg->receiver_id;
  chat_log_t *log = &g_app_state.chat_logs[user_id];

  // Insert at head
  log->entries[log->head].msg = *msg;
  log->head = (log->head + 1) % CHAT_LOG_SIZE;

  if (log->count < CHAT_LOG_SIZE) {
    log->count++;
  }

  g_app_state.new_message_flag = true;
}

/**
 * @brief Retrieve last N messages for a user
 */
int app_state_get_messages(int user_id, chat_entry_t *out_entries,
                           int max_entries) {
  if (user_id < 0 || user_id >= MAX_USERS) {
    return 0;
  }

  chat_log_t *log = &g_app_state.chat_logs[user_id];
  int to_copy = (log->count < max_entries) ? log->count : max_entries;

  for (int i = 0; i < to_copy; i++) {
    int idx = (log->head - log->count + i + CHAT_LOG_SIZE) % CHAT_LOG_SIZE;
    out_entries[i] = log->entries[idx];
  }

  return to_copy;
}
