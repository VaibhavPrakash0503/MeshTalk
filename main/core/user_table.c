#include "user_table.h"
#include <stdint.h>

user_t user_table[MAX_USERS] = {{2, "Nishita", 0x0002}, {3, "Ishita", 0x0003}};

uint16_t get_unicast_addr(uint16_t reciever_id) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].user_id == reciever_id) {
      return user_table[i].unicast_addr;
    }
  }
  return 0;
}
