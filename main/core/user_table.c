#include "user_table.h"
#include "constants.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "USER_TABLE";
static const char *NVS_NAMESPACE = "meshtalk";
static const char *NVS_USER_COUNT_KEY = "user_count";
static const char *NVS_USER_KEY_PREFIX = "user_";

user_t user_table[MAX_USERS] = {0};

// Save current user table to NVS
esp_err_t user_table_save_to_nvs(void) {
  nvs_handle_t nvs_handle;
  esp_err_t err;

  // Open NVS namespace
  err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle", esp_err_to_name(err));
    return err;
  }

  // Count valid users
  int valid_count = 0;
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid) {
      valid_count++;
    }
  }

  // Save user count
  err = nvs_set_i32(nvs_handle, NVS_USER_COUNT_KEY, valid_count);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error saving user count: %s", esp_err_to_name(err));
    nvs_close(nvs_handle);
    return err;
  }

  // Structure to save user data
  typedef struct {
    char username[USERNAME_MAX_LEN];
    uint16_t unicast_addr;
    bool valid;
  } nvs_user_entry_t;

  // Save each valid user
  int saved_count = 0;
  for (int i = 0; i < MAX_USERS && saved_count < valid_count; i++) {
    if (user_table[i].valid) {
      char key[32];
      snprintf(key, sizeof(key), "%s%d", NVS_USER_KEY_PREFIX, saved_count);

      nvs_user_entry_t entry;
      strncpy(entry.username, user_table[i].username, USERNAME_MAX_LEN);
      entry.username[USERNAME_MAX_LEN - 1] = '\0';
      entry.unicast_addr = user_table[i].unicast_addr;
      entry.valid = user_table[i].valid;

      size_t required_size = sizeof(nvs_user_entry_t);
      err = nvs_set_blob(nvs_handle, key, &entry, required_size);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving user %d: %s", saved_count,
                 esp_err_to_name(err));
      } else {
        ESP_LOGD(TAG, "Saved user %s (0x%04X) to NVS", entry.username,
                 entry.unicast_addr);
      }
      saved_count++;
    }
  }

  // Commit changes
  err = nvs_commit(nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error committing NVS changes: %s", esp_err_to_name(err));
  } else {
    ESP_LOGI(TAG, "Successfully saved %d users to NVS", saved_count);
  }

  nvs_close(nvs_handle);
  return err;
}

// Load user table from NVS
esp_err_t user_table_load_from_nvs(void) {
  nvs_handle_t nvs_handle;
  esp_err_t err;

  // Open NVS namespace
  err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "No existing user data found in NVS: %s",
             esp_err_to_name(err));
    return err;
  }

  // Get user count
  int32_t user_count = 0;
  err = nvs_get_i32(nvs_handle, NVS_USER_COUNT_KEY, &user_count);
  if (err != ESP_OK || user_count <= 0) {
    ESP_LOGI(TAG, "No users found in NVS");
    nvs_close(nvs_handle);
    return err;
  }

  ESP_LOGI(TAG, "Loading %d users from NVS", (int)user_count);

  // Clear current user table
  memset(user_table, 0, sizeof(user_table));

  // Structure to load user data
  typedef struct {
    char username[USERNAME_MAX_LEN];
    uint16_t unicast_addr;
    bool valid;
  } nvs_user_entry_t;

  // Load each user
  int loaded_count = 0;
  for (int i = 0; i < user_count && loaded_count < MAX_USERS; i++) {
    char key[32];
    snprintf(key, sizeof(key), "%s%d", NVS_USER_KEY_PREFIX, i);

    nvs_user_entry_t entry;
    size_t required_size = sizeof(nvs_user_entry_t);

    err = nvs_get_blob(nvs_handle, key, &entry, &required_size);
    if (err == ESP_OK && loaded_count < MAX_USERS) {
      // Add user to table
      user_table[loaded_count].valid = true;
      user_table[loaded_count].unicast_addr = entry.unicast_addr;
      strncpy(user_table[loaded_count].username, entry.username,
              USERNAME_MAX_LEN);
      user_table[loaded_count].username[USERNAME_MAX_LEN - 1] = '\0';

      ESP_LOGD(TAG, "Loaded user %s (0x%04X) from NVS at index %d",
               entry.username, entry.unicast_addr, loaded_count);
      loaded_count++;
    } else {
      ESP_LOGW(TAG, "Failed to load user %d: %s", i, esp_err_to_name(err));
    }
  }

  nvs_close(nvs_handle);
  ESP_LOGI(TAG, "Successfully loaded %d users from persistent storage",
           loaded_count);
  return ESP_OK;
}

// Clear user table (both RAM and NVS)
void user_table_clear(void) {
  memset(user_table, 0, sizeof(user_table));

  // Clear NVS data
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err == ESP_OK) {
    nvs_erase_all(nvs_handle);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Cleared user table and NVS storage");
  }
}

// Modified user_table_set with NVS persistence
bool user_table_set(const char *username, uint16_t unicast_addr) {
  if (!username || strlen(username) == 0) {
    ESP_LOGW(TAG, "Invalid username provided");
    return false;
  }

  // Check if address already exists -> update if name is different
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && user_table[i].unicast_addr == unicast_addr) {
      // If name is different, update it
      if (strncmp(user_table[i].username, username, USERNAME_MAX_LEN) != 0) {
        strncpy(user_table[i].username, username, USERNAME_MAX_LEN - 1);
        user_table[i].username[USERNAME_MAX_LEN - 1] = '\0';
        ESP_LOGI(TAG, "Updated user at index %d: %s (0x%04X)", i, username,
                 unicast_addr);

        // Save to NVS
        user_table_save_to_nvs();
      }
      return true; // Address already exists (updated if needed)
    }
  }

  // Otherwise add a new entry
  for (int i = 0; i < MAX_USERS; i++) {
    if (!user_table[i].valid) {
      user_table[i].valid = true;
      user_table[i].unicast_addr = unicast_addr;
      strncpy(user_table[i].username, username, USERNAME_MAX_LEN - 1);
      user_table[i].username[USERNAME_MAX_LEN - 1] = '\0';
      ESP_LOGI(TAG, "Added new user at index %d: %s (0x%04X)", i, username,
               unicast_addr);

      // Save to NVS
      user_table_save_to_nvs();
      return true;
    }
  }

  // Table full
  ESP_LOGW(TAG, "User table full, cannot add %s", username);
  return false;
}

uint16_t user_table_get_addr(const char *username) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && strncmp(user_table[i].username, username,
                                       sizeof(user_table[i].username)) == 0) {
      return user_table[i].unicast_addr;
    }
  }
  return 0; // Not found
}

const char *user_table_get_name(uint16_t unicast_addr) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && user_table[i].unicast_addr == unicast_addr) {
      return user_table[i].username;
    }
  }
  return NULL; // Not found
}

void user_table_print(void) {
  printf("=== User Table ===\n");
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid) {
      printf("Name: %s | Addr: 0x%04X\n", user_table[i].username,
             user_table[i].unicast_addr);
    }
  }
}

int user_table_find_index_by_addr(uint16_t addr) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && user_table[i].unicast_addr == addr) {
      return i;
    }
  }
  return -1; // not found
}

int user_table_find_index_by_name(const char *username) {
  if (!username)
    return -1;

  for (int i = 0; i < MAX_USERS; i++) {
    if (user_table[i].valid && strcmp(user_table[i].username, username) == 0) {
      return i;
    }
  }
  return -1; // not found
}
