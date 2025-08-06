# Contributing to MeshTalk

Thank you for your interest in contributing to **MeshTalk** – a lightweight BLE Mesh chat system for ESP32!

To maintain consistency and quality across the project, please follow the guidelines below.

---

## 📁 Project Structure Guidelines

- **Organize code into modules**: All functionality should be structured under `main/`, `core/`, `logic/`, `mesh/`, `ui/`, and `include/`.
- **Use header files**: All shared functions, macros, and constants must be declared in the `include/` directory.
- **Driver code**: Keep all hardware-specific logic modular inside the appropriate component folder.

---

## 🧠 Code Style

- **Language**: All code should be in C (ESP-IDF style).
- **Indentation**: Use 4 spaces per indentation level.
- **Naming**:
  - Functions: `snake_case`, e.g., `send_encrypted_message()`
  - Constants/Macros: `ALL_UPPERCASE_WITH_UNDERSCORES`
  - Files: `lower_snake_case.c/h`

- **Comments**: Use `//` for single-line and `/** */` for function/method documentation.
- **Function Documentation** (Use Doxygen style):

```c
/**
 * @brief Encrypt and send a message to a given node.
 *
 * @param dst_node_addr Address of the recipient node.
 * @param message Pointer to the message data.
 * @param len Length of the message.
 * @return esp_err_t
 */
esp_err_t send_encrypted_message(uint16_t dst_node_addr, const uint8_t *message, size_t len);

