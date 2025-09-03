#pragma once
#include <stdint.h>

/**
 * @brief Structure holding node configuration info
 */
typedef struct {
  char name[32];    ///< Human-readable node name
  uint16_t address; ///< Provisioned unicast address
} node_config_t;

/**
 * @brief Initialize node configuration with a fixed name.
 * @param name Node name (set at compile time or runtime).
 */
void node_config_init(const char *name);

/**
 * @brief Save assigned unicast address after provisioning.
 * @param addr The unicast address provisioner gave this node.
 */
void node_config_set_address(uint16_t addr);

/**
 * @brief Get current node configuration.
 * @return Pointer to global node config struct.
 */
const node_config_t *node_config_get(void);
