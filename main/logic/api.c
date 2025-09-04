#include "api.h"
#include "app_state.h"
#include "chat_log.h"
#include "constants.h"
#include "esp_log_timestamp.h"
#include "esp_timer.h"
#include "message_handler.h"
#include "message_struct.h"
#include "node_config.h"
#include "user_table.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Keep UI callback pointer
static ui_receive_cb_t ui_cb = NULL;

/**
 * Initialize API and register UI callback.
 */
void api_init(ui_receive_cb_t cb) { ui_cb = cb; }

/**
 * Convert plain text to MeshMessage and send via message_handler.
 */
void api_send_text(const char *receiver_name, const char *msg) {

  int idx = user_table_find_index_by_name(receiver_name);
  if (idx >= 0) {
    chat_log_add(idx, msg, true); // true = outgoing
  }

  MeshMessage m;
  memset(&m, 0, sizeof(MeshMessage));

  m.type = 1; // 1 = text message
  m.timestamp = esp_timer_get_time();

  const node_config_t *cfg = node_config_get();
  strncpy(m.sender_name, cfg->name, USERNAME_MAX_LEN);
  m.addr = cfg->address;

  size_t msg_len = strlen(msg);
  if (msg_len > MAX_MESSAGE_LEN) {
    msg_len = MAX_MESSAGE_LEN; // truncate
  }
  m.payload_len = msg_len;
  memcpy(m.payload, msg, msg_len);

  uint16_t receiver_add = user_table_get_addr(receiver_name);

  message_handler_send(&m, receiver_add);
}

/**
 * Called by message_handler when a new message is received.
 * Converts MeshMessage → (sender_id, text) → UI callback.
 */
void api_on_message_received(const MeshMessage *m) {
  uint16_t sender_addr = m->addr;
  const char *sender_name = m->sender_name;

  // --- Handle broadcast messages ---
  if (m->type == 2) {                         // 2 = broadcast
    user_table_set(sender_name, sender_addr); // add/update user table
    return;                                   // no further processing needed
  }

  // --- Normal chat message ---
  int idx = user_table_find_index_by_addr(sender_addr);
  if (idx >= 0) {
    if (g_app_state.selected_user == 0x0000 ||
        g_app_state.selected_user != sender_addr) {
      g_app_state.new_message_flags[idx] = true; // mark new message
    }
  }

  // 1. Copy payload into buffer (if any)
  static char buffer[MAX_MESSAGE_LEN + 1];
  size_t copy_len =
      (m->payload_len > MAX_MESSAGE_LEN) ? MAX_MESSAGE_LEN : m->payload_len;
  if (copy_len > 0) {
    memcpy(buffer, m->payload, copy_len);
  }
  buffer[copy_len] = '\0';

  // <-- store in chat log -->
  if (idx >= 0 && copy_len > 0) {
    chat_log_add(idx, buffer, false);
  }

  // 2. Notify UI if registered
  if (ui_cb) {
    ui_cb(sender_name, buffer);
  }
}

void api_broadcast_addr(void) {
  const node_config_t *cfg = node_config_get();
  MeshMessage msg;

  msg.type = 2; // broadcast
  msg.timestamp = esp_timer_get_time();
  strncpy(msg.sender_name, cfg->name, USERNAME_MAX_LEN);
  msg.addr = cfg->address; // directly store the address
  msg.payload_len = 0;
  // Store own address in payload (little-endian)

  message_handler_broadcast(&msg); // hand off to mesh
}

int api_get_contact_names(char names[][USERNAME_MAX_LEN], int max_contacts) {
  if (!names || max_contacts <= 0)
    return 0;

  int count = 0;
  for (int i = 0; i < MAX_USERS && count < max_contacts; i++) {
    if (user_table[i].valid) {
      strncpy(names[count], user_table[i].username, USERNAME_MAX_LEN);
      names[count][USERNAME_MAX_LEN - 1] = '\0';
      count++;
    }
  }
  return count;
}

int api_get_chat_log(const char *username, char out_buf[][MAX_MESSAGE_LEN + 1],
                     int max_msgs) {
  if (!username || !out_buf || max_msgs <= 0)
    return 0;

  int user_idx = user_table_find_index_by_name(username);
  if (user_idx < 0)
    return 0;

  return chat_log_get(user_idx, out_buf, max_msgs);
}
