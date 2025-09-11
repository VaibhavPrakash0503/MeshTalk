# MeshTalk

MeshTalk is a BLE Mesh–based chat application built for the **ESP32** platform using the **ESP-IDF framework**.  
It allows multiple nodes to communicate over a Bluetooth Mesh network using a simple text-based protocol,  
with support for hardware input/output (joystick + button for navigation, SSD1306 OLED display for UI).  

Provisioning is done using the **nRF Mesh mobile application** (Nordic Semiconductor).  

---

## ✨ Features
- **BLE Mesh Chat**: Send and receive short text messages across mesh nodes.
- **Vendor Model**: Custom vendor-specific model for chat message handling.
- **Provisioning**: Static and dynamic provisioning supported.
  - ✅ Dynamic provisioning using the **nRF Mesh app**.
- **Hardware UI**:
  - SSD1306 OLED screen for displaying messages.
  - PS2 joystick with button for user input and navigation.
- **Message Enhancements**:
  - Timestamps using `esp_timer_get_time()`.
  - Duplicate detection with Bloom filter.
  - Compact binary protocol (1-byte type fields).
  - Aggregated messages for efficiency.

---

### 🖥️ Display Driver
This project uses the [nopnop2002/esp-idf-ssd1306](https://github.com/nopnop2002/esp-idf-ssd1306) driver  
via ESP-IDF's `managed_components` for SSD1306 OLED support.

## 🔧 Hardware Setup
- **ESP32 development boards** (≥2 nodes for chat).
- **SSD1306 OLED** (I2C pins: `SDA`, `SCL`).
- **PS2 Joystick module** (VRx, VRy, SW pins).
- Breadboard + jumper wires.

**Connections**:
- OLED:
  - `VDD` → 3.3V
  - `GND` → GND
  - `SCL` → ESP32 GPIO22 (default I2C SCL)
  - `SDA` → ESP32 GPIO21 (default I2C SDA)
- Joystick:
  - `+5V` → 3.3V (⚠️ many modules accept 3.3V)
  - `GND` → GND
  - `VRx` → ADC pin (e.g., GPIO34)
  - `VRy` → ADC pin (e.g., GPIO35)
  - `SW`  → GPIO (e.g., GPIO32)

---

## 📲 Provisioning with nRF Mesh App
1. Install the **nRF Mesh app** from Google Play or App Store.
2. Power up your ESP32 nodes with the MeshTalk firmware flashed.
3. Open the app and scan for unprovisioned devices.
4. Select your ESP32 device and **provision** it:
   - Add a **NetKey** and **AppKey**.
   - Bind the AppKey to the vendor model.
5. After provisioning, you can send messages between nodes.

---

## ▶️ Getting Started

### Requirements
- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html)
- Python 3.x
- nRF Mesh mobile app (for provisioning)

### Build & Flash
```bash
# Set up ESP-IDF environment
. $HOME/esp/esp-idf/export.sh

# Build
idf.py build

# Flash to ESP32
idf.py -p /dev/ttyUSB0 flash monitor
