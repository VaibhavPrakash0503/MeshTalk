#include "api.h"
#include "app_state.h"
#include "message_handler.h"
#include "message_struct.h"
#include "node_config.h"
#include "user_table.h"
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
void api_send_text(uint16_t receiver_id, const char *msg) {
  MeshMessage m;
  memset(&m, 0, sizeof(MeshMessage));

  m.type = 1;                 // 1 = text message
  m.sender_id = NODE_USER_ID; // Todo get from config/local table
  m.receiver_id = receiver_id;
  m.timestamp = (uint64_t)time(NULL);

  size_t msg_len = strlen(msg);
  if (msg_len > MAX_MESSAGE_LEN) {
    msg_len = MAX_MESSAGE_LEN; // truncate
  }
  m.payload_len = msg_len;
  memcpy(m.payload, msg, msg_len);

  uint16_t receiver_add = get_unicast_addr(receiver_id);

  message_handler_send(&m, receiver_add);
}

/**
 * Called by message_handler when a new message is received.
 * Converts MeshMessage → (sender_id, text) → UI callback.
 */
void api_on_message_received(const MeshMessage *m) {

  int sender = m->sender_id;

  // Update new message flag if chat with this sender is not open
  if (g_app_state.selected_user != sender) {
    g_app_state.new_message_flags[sender] = true;
  }

  // 2. Notify UI if registered
  if (ui_cb) {
    static char buffer[MAX_MESSAGE_LEN + 1];
    size_t copy_len =
        (m->payload_len > MAX_MESSAGE_LEN) ? MAX_MESSAGE_LEN : m->payload_len;
    memcpy(buffer, m->payload, copy_len);
    buffer[copy_len] = '\0';

    ui_cb(m->sender_id, buffer);
  }
}
