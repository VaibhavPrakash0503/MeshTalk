#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_CHAT_PER_USER 6
#define MAX_MESSAGE_LEN 64

typedef struct {
  char messages[MAX_CHAT_PER_USER][MAX_MESSAGE_LEN + 1]; // Last 5 messages
  int head;  // Points to the next slot to write (circular buffer)
  int count; // Number of messages stored (<= MAX_CHAT_PER_USER)
} user_chat_log_t;

/**
 * Initialize chat log (clear all entries)
 */
void chat_log_init(void);

/**
 * Store a new chat message for a user index
 * @param user_idx Index in user_table
 * @param msg      Message string (null-terminated)
 */
void chat_log_add(int user_idx, const char *msg, bool outgoing);

/**
 * Retrieve all stored chat messages for a user index
 * @param user_idx Index in user_table
 * @param out_buf  Array of strings to copy messages into
 * @param max_msgs Maximum number of messages out_buf can hold
 * @return Number of messages copied
 */
int chat_log_get(int user_idx, char out_buf[][MAX_MESSAGE_LEN + 1],
                 int max_msgs);
