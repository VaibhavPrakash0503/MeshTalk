#include "node_config.h"
#include <string.h>

static node_config_t node_cfg;

void node_config_init(const char *name) {
  memset(&node_cfg, 0, sizeof(node_cfg));
  strncpy(node_cfg.name, name, sizeof(node_cfg.name) - 1);
  node_cfg.address = 0x0000; // not yet provisioned
}

void node_config_set_address(uint16_t addr) { node_cfg.address = addr; }

const node_config_t *node_config_get(void) { return &node_cfg; }
