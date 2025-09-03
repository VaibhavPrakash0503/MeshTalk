#include "api.h"
#include "app_state.h"
#include "chat_log.h"
#include "message_handler.h"
#include "message_struct.h"
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
  MeshMessage m;
  memset(&m, 0, sizeof(MeshMessage));

  m.type = 1; // 1 = text message
  m.timestamp = (uint64_t)time(NULL);

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
void api_on_message_received(uint16_t sender_addr, const MeshMessage *m) {

  const char *sender_name = user_table_get_name(sender_addr);

  // --- If chat with this sender is not currently open, mark new message ---
  int idx = user_table_find_index_by_addr(sender_addr);
  if (idx >= 0) {
    if (g_app_state.selected_user == 0x0000 ||
        g_app_state.selected_user != sender_addr) {
      g_app_state.new_message_flags[idx] = true; // mark flag for this user
    }
  }

  // 1. Copy payload into buffer
  static char buffer[MAX_MESSAGE_LEN + 1];
  size_t copy_len =
      (m->payload_len > MAX_MESSAGE_LEN) ? MAX_MESSAGE_LEN : m->payload_len;
  memcpy(buffer, m->payload, copy_len);
  buffer[copy_len] = '\0';

  // <-- store in chat log -->
  if (idx >= 0) {
    chat_log_add(idx, buffer);
  }

  // 2. Notify UI if registered
  if (ui_cb) {
    ui_cb(sender_name, buffer);
  }
}
