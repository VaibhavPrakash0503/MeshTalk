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

void chat_log_add(int user_idx, const char *msg) {
  if (user_idx < 0 || user_idx >= MAX_USERS || !msg)
    return;

  user_chat_log_t *log = &chat_logs[user_idx];
  strncpy(log->messages[log->head], msg, MAX_MESSAGE_LEN);
  log->messages[log->head][MAX_MESSAGE_LEN] = '\0'; // ensure null termination

  log->head = (log->head + 1) % MAX_CHAT_PER_USER;
  if (log->count < MAX_CHAT_PER_USER)
    log->count++;
}

int chat_log_get(int user_idx, char out_buf[][MAX_MESSAGE_LEN + 1],
                 int max_msgs) {
  if (user_idx < 0 || user_idx >= MAX_USERS || !out_buf || max_msgs <= 0)
    return 0;

  user_chat_log_t *log = &chat_logs[user_idx];
  int copied = 0;
  int start = (log->count == MAX_CHAT_PER_USER) ? log->head : 0;

  for (int i = 0; i < log->count && copied < max_msgs; i++) {
    int idx = (start + i) % MAX_CHAT_PER_USER;
    strncpy(out_buf[copied], log->messages[idx], MAX_MESSAGE_LEN);
    out_buf[copied][MAX_MESSAGE_LEN] = '\0';
    copied++;
  }
  return copied;
}
