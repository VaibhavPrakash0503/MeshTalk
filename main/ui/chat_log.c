#include "chat_log.h"
#include "user_table.h"
#include <stdio.h>
#include <string.h>

static user_chat_log_t chat_logs[MAX_USERS];

void chat_log_init(void) {
  for (int i = 0; i < MAX_USERS; i++) {
    chat_logs[i].head = 0;
    chat_logs[i].count = 0;
    for (int j = 0; j < MAX_CHAT_PER_USER; j++) {
      chat_logs[i].messages[j][0] = '\0';
    }
  }
}

void chat_log_add(int user_idx, const char *msg, bool outgoing) {
  if (user_idx < 0 || user_idx >= MAX_USERS || !msg)
    return;

  user_chat_log_t *log = &chat_logs[user_idx];
  if (outgoing) {
    snprintf(log->messages[log->head], MAX_MESSAGE_LEN, "[Y] %s", msg);
  } else {
    strncpy(log->messages[log->head], msg, MAX_MESSAGE_LEN);
    log->messages[log->head][MAX_MESSAGE_LEN - 1] =
        '\0'; // ensure null termination
  }

  log->head = (log->head + 1) % MAX_CHAT_PER_USER;
  if (log->count < MAX_CHAT_PER_USER)
    log->count++;
}

int chat_log_get(int user_idx, char out_buf[][MAX_MESSAGE_LEN + 1],
                 int max_msgs) {
  if (user_idx < 0 || user_idx >= MAX_USERS || !out_buf || max_msgs <= 0)
    return 0;

  user_chat_log_t *log = &chat_logs[user_idx];

  if (log->count == 0)
    return 0;

  // Number of messages to return (min of max_msgs or log->count)
  int num_to_copy = (log->count < max_msgs) ? log->count : max_msgs;

  // Calculate start index for last N messages
  int start_idx;
  if (log->count < MAX_CHAT_PER_USER) {
    start_idx = log->count - num_to_copy;
  } else {
    // log full, use circular buffer math
    start_idx =
        (log->head + MAX_CHAT_PER_USER - num_to_copy) % MAX_CHAT_PER_USER;
  }

  // Copy in order
  for (int i = 0; i < num_to_copy; i++) {
    int idx = (start_idx + i) % MAX_CHAT_PER_USER;
    strncpy(out_buf[i], log->messages[idx], MAX_MESSAGE_LEN);
    out_buf[i][MAX_MESSAGE_LEN] = '\0';
  }

  return num_to_copy;
}
