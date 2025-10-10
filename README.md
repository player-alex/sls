# 🚪 Smart Door Lock System

> 🔐 IoT-enabled biometric door lock powered by ESP32-S3 and Microsoft Azure

A personal IoT project (university assignment) featuring multiple authentication methods (fingerprint or PIN), cloud telemetry, and intelligent power management. Built on ESP32-S3 with real-time Azure IoT Hub integration.

[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-blue.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Azure IoT](https://img.shields.io/badge/Azure-IoT%20Hub-0078D4.svg)](https://azure.microsoft.com/en-us/services/iot-hub/)
[![FreeRTOS](https://img.shields.io/badge/FreeRTOS-Enabled-green.svg)](https://www.freertos.org/)

---

## ✨ Features

### 🔒 Security & Authentication
- **🖐️ Biometric Authentication** - Optical fingerprint sensor (JM-101B) with 2-stage enrollment
- **🔢 PIN Authentication** - Secure password system (up to 64 digits) stored in NVS flash
- **🚨 Auto-Lockdown** - 30-second security lockdown after 3 failed authentication attempts
- **🔄 Password Management** - On-device password change with double verification

### ☁️ Cloud & IoT
- **📡 Azure IoT Hub** - Real-time device-to-cloud telemetry
- **🔌 Auto-Provisioning** - Device Provisioning Service (DPS) with symmetric key auth
- **📊 Live Monitoring** - Integration with Azure IoT Central dashboards
- **🔔 Event Streaming** - 10 telemetry events (door status, security alerts, system events)
- **🔐 Secure Communication** - MQTT over TLS 1.2 with Azure CA certificates

### ⚡ Power Management
- **💤 Deep Sleep Mode** - Ultra-low power consumption after 30s inactivity
- **⏰ RTC Wake-Up** - Wake from keypad, fingerprint touch, or buttons via RTC GPIO
- **🔌 Smart Power Control** - PNP transistor-switched peripherals (on-demand activation)
- **🔋 Battery Optimized** - Intelligent module power gating for extended operation

### 🎵 User Experience
- **🔊 Voice Feedback** - I2S-based audio prompts in Korean (extensible to other languages)
- **🎹 Matrix Keypad** - 3x4 tactile keypad with debouncing
- **🚪 Auto-Lock** - Automatic door lock after 5 seconds
- **⏱️ Activity Timeout** - 30-second inactivity timer
- **🔴 Emergency Access** - Physical button override for manual control

---

## 🛠️ Hardware Requirements

### Main Components

| Component | Model/Spec | Purpose |
|-----------|------------|---------|
| 🧠 **Microcontroller** | ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM) | Main controller with WiFi |
| 🖐️ **Fingerprint Reader** | JM-101B (UART) | Biometric authentication |
| 🎹 **Keypad** | 3x4 Matrix Keypad | PIN entry |
| 🔊 **Audio Amplifier** | MAX98357A (I2S) | Voice feedback |
| ⚙️ **Motor Driver** | L298N Dual H-Bridge | Lock/unlock mechanism |
| 🔩 **DC Motor** | Geared DC Motor | Physical lock actuator |
| ⚡ **PNP Transistors** | ≥350mA current rating × 3 | Peripheral power control |
| 🔘 **Buttons** | Tactile push buttons × 3 | Reset, Door Control, Enroll |

### Optional Components
- 👁️ **PIR Sensor** - Motion detection for auto-wake (GPIO 0)
- 🔆 **LED Indicators** - Visual status feedback

---

## 📡 Azure Services Setup

### Required Services
1. **☁️ Azure IoT Hub** - Device messaging and telemetry
2. **🔄 Device Provisioning Service (DPS)** - Automated device registration
3. **📊 Azure IoT Central** *(Optional)* - Dashboard and monitoring

### Optional Services
4. **📱 Notification Hubs** - Mobile push notifications
5. **🔥 Firebase Cloud Messaging** - Cross-platform alerts

### Configuration Steps

#### 1️⃣ Device Provisioning Service
1. Create a DPS instance in Azure Portal
2. Create an enrollment group with **Symmetric Key** attestation
3. Copy the following values to `main/config.h`:
   ```c
   #define AZURE_IOT_DPS_ID_SCOPE          "0neXXXXXXXX"
   #define AZURE_IOT_DPS_SYMMETRIC_KEY     "your-primary-key-here"
   ```

#### 2️⃣ IoT Central (Optional)
1. Create an IoT Central application
2. Create a device template with the following telemetry schema:
   ```json
   {
     "status": "integer",
     "desc": "string"
   }
   ```
3. Copy the model ID to config.h:
   ```c
   #define AZURE_IOT_DPS_MODEL_ID          "dtmi:yourcompany:SmartLock;1"
   ```

📚 **Authentication Guide**: [Device authentication concepts in IoT Central](https://learn.microsoft.com/en-us/azure/iot-central/core/concepts-device-authentication)

---

## 🔌 Pin Configuration

### ESP32-S3 GPIO Mapping

#### Fingerprint Reader (JM-101B)
- `GPIO 42` - Power Control (PNP Transistor)
- `GPIO 18` - UART TX
- `GPIO 17` - UART RX
- `GPIO 9` - Touch Sensor RX
- `GPIO 10` - Touch Sensor Power

#### Keypad (3×4 Matrix)
- **Columns**: `GPIO 4, 5, 6`
- **Rows**: `GPIO 11, 12, 13, 14`

#### Audio (MAX98357A I2S)
- `GPIO 40` - Power Control (PNP Transistor)
- `GPIO 2` - BCLK (Bit Clock)
- `GPIO 1` - WS (Word Select)
- `GPIO 41` - DOUT (Data Out)

#### Motor Driver (L298N)
- `GPIO 7` - Power Control (PNP Transistor)
- `GPIO 39` - ENA/IN1
- `GPIO 38` - ENA/IN2

#### Control Buttons
- `GPIO 8` - Reset Button (Factory Reset)
- `GPIO 15` - Door Switch (Manual Override)
- `GPIO 16` - Enroll Button (Fingerprint Enrollment)

#### Reserved Pins
⚠️ **Do NOT use**: `GPIO 33-37` (Reserved for PSRAM)

---

## 🚀 Getting Started

### Prerequisites
- ✅ ESP-IDF v5.0 or later
- ✅ Azure account with IoT Hub + DPS
- ✅ WiFi network (2.4GHz)

### Build & Flash

1. **Clone the repository**
   ```bash
   git clone https://github.com/player-alex/sls.git
   cd sls
   ```

2. **Configure settings** in `main/config.h`:
   ```c
   // Device Identity
   #define DEV_ID                "your-device-id"

   // WiFi Credentials
   #define WIFI_AP_SSID         "your-wifi-ssid"
   #define WIFI_AP_PASSWORD     "your-wifi-password"

   // Azure Credentials (see Azure Services Setup)
   #define AZURE_IOT_DPS_ID_SCOPE          "your-id-scope"
   #define AZURE_IOT_DPS_SYMMETRIC_KEY     "your-symmetric-key"
   #define AZURE_IOT_DPS_MODEL_ID          "your-model-id"
   ```

3. **Build and flash**
   ```bash
   idf.py build
   idf.py -p COM3 flash monitor  # Replace COM3 with your port
   ```

### First Boot
1. 🔌 Device powers on
2. 📶 Connects to WiFi
3. ⏰ Syncs time via SNTP
4. ☁️ Provisions with Azure DPS
5. 🔗 Connects to IoT Hub
6. ✅ Ready for operation!

**Default Password**: `0000`

---

## 🎮 Usage Guide

### 🔓 Unlocking the Door

#### Method 1: Password
1. Enter your password on the keypad (e.g., `1234`)
2. Press `*` to verify
3. ✅ Door unlocks if password is correct
4. 🔊 Audio: "열렸습니다" (Opened)

#### Method 2: Fingerprint
1. Touch the fingerprint sensor
2. Place your enrolled finger
3. ✅ Door unlocks if fingerprint matches
4. 🔊 Audio: "열렸습니다" (Opened)

### 🔒 Locking the Door
- **Auto-Lock**: Door automatically locks 5 seconds after opening
- **Manual Lock**: Press Door Switch button (GPIO 15)

### 🔑 Changing Password
1. Enter current password
2. Press `#` (enters password change mode)
3. 🔊 System beeps 2 times
4. Enter new password + press `*`
5. 🔊 System beeps 3 times
6. Re-enter new password + press `*`
7. ✅ Password changed
8. 🔊 Audio: "등록되었습니다" (Enrolled)

### 🖐️ Enrolling Fingerprint
1. Press Enroll Button (GPIO 16)
2. 🔊 System beeps 3 times (enrollment mode)
3. Place finger on sensor → Remove → Place again
4. ✅ Fingerprint enrolled
5. 🔊 Audio: "등록되었습니다" (Enrolled) or "등록에 실패했습니다" (Failed)

### 🔴 Factory Reset
1. Press and hold Reset Button (GPIO 8)
2. All passwords and fingerprints erased
3. System restarts with default password `0000`

### 🚨 Lockdown Mode
- Triggered after **3 failed authentication attempts** (password or fingerprint)
- System locks for **30 seconds**
- 🚨 Security siren plays
- 📡 Telemetry alert sent to Azure
- All authentication blocked during lockdown

---

## 🎵 Audio System

### Audio Pipeline
```
TTS (Text-to-Speech) → MP3 → WAV → wav2code → Embedded C Arrays
```

### Available Audio Prompts (Korean)
| File | Korean | English | Usage |
|------|--------|---------|-------|
| `audio_data_opened.h` | 열렸습니다 | Opened | Door unlocked |
| `audio_data_closed.h` | 닫혔습니다 | Closed | Door locked |
| `audio_data_enrolled.h` | 등록되었습니다 | Enrolled | Fingerprint/password enrolled |
| `audio_data_enrollment_failed.h` | 등록에 실패했습니다 | Enrollment Failed | Enrollment error |
| `audio_data_repeat_again.h` | 다시 입력해주세요 | Please try again | Authentication failed |
| `audio_data_beep.h` | 🔔 | Beep | Key press feedback |
| `audio_data_siren.h` | 🚨 | Siren | Security lockdown |

### Audio Credits
- 🎵 Sound effects from [Pixabay](https://pixabay.com)
- 🎙️ Voice prompts: Custom TTS generation

### Adding New Audio
1. Generate MP3 using TTS service
2. Convert to WAV (16-bit PCM, mono)
3. Use `wav2code` tool to create C array
4. Add header file to `main/audio/data/`
5. Register in `metadata.cpp`

---

## 📊 Telemetry Events

### Event Codes

| Code | Event Name | Description | Trigger |
|------|------------|-------------|---------|
| 1 | `Opened` | Door unlocked | Successful authentication |
| 2 | `Closed` | Door locked | Auto-lock or manual lock |
| 3 | `PasswordMismatch` | Wrong password entered | Failed password attempt |
| 4 | `FingerprintMismatch` | Unknown fingerprint | Failed fingerprint scan |
| 5 | `LockdownCausePasswordMismatch` | Security lockdown | 3 password failures |
| 6 | `LockdownCauseFingerprintMismatch` | Security lockdown | 3 fingerprint failures |
| 7 | `PasswordChanged` | Password updated | Successful password change |
| 8 | `StartFingerprintEnrollment` | Enrollment started | Enroll button pressed |
| 9 | `FingerprintEnrolled` | Fingerprint added | Successful enrollment |
| 10 | `FingerprintEnrollmentFailed` | Enrollment error | Failed enrollment |
| 11 | `NotEnoughBattery` | Low battery warning | Battery threshold (reserved) |
| 12 | `SystemBooted` | Device started | Power-on or reset |

### JSON Format
```json
{
  "status": 1,
  "desc": "Door opened via fingerprint"
}
```

### Azure IoT Hub Configuration
- **Protocol**: MQTT over TLS 1.2
- **QoS**: 1 (At least once delivery)
- **Port**: 8883
- **Telemetry Interval**: 3 seconds
- **Queue Size**: 100 messages
- **Retry Logic**: 5 attempts with 500ms interval

---

## 🏗️ Architecture

### System Overview
```
┌─────────────────────────────────────────────────────────────┐
│                     ESP32-S3 FreeRTOS                       │
├─────────────────────────────────────────────────────────────┤
│  🎯 Main Application (app_main.cpp)                         │
│     ├── Task Manager (8 concurrent tasks)                   │
│     ├── State Machine (Door/System Status)                  │
│     └── Event Handlers (Buttons/Keypad/Fingerprint)         │
├─────────────────────────────────────────────────────────────┤
│  📦 Core Modules                                            │
│     ├── 🔷 Azure IoT Integration (DPS, IoT Hub, Telemetry)  │
│     ├── 🖐️ Fingerprint Auth (JM-101B Driver)               │
│     ├── 🔊 I2S Audio Controller (MAX98357A)                 │
│     ├── 🎹 Keypad Scanner (3x4 Matrix)                      │
│     ├── ⚙️ Motor Controller (L298N)                         │
│     └── 📶 WiFi Station Manager                             │
├─────────────────────────────────────────────────────────────┤
│  🛠️ Helper/Utility Layer                                    │
│     ├── 💾 NVS Storage (Password persistence)              │
│     ├── ⏰ SNTP Time Sync (TLS requirement)                 │
│     └── 🔄 Cancellation Tokens (Async control)              │
├─────────────────────────────────────────────────────────────┤
│  🔧 ESP-IDF HAL (GPIO, UART, I2S, WiFi, NVS, RTC)          │
└─────────────────────────────────────────────────────────────┘
```

### FreeRTOS Tasks

| Task | Priority | Stack | Interval | Purpose |
|------|----------|-------|----------|---------|
| `tsk_scan_btns` | 0 | 8KB | 100ms | Button scanning |
| `tsk_scan_keys` | 0 | 8KB | 10ms | Keypad matrix scan |
| `tsk_scan_fp` | 0 | 8KB | 250ms | Fingerprint touch detection |
| `tsk_upd_status` | 0 | 8KB | 10ms | Status monitor & timers |
| `tsk_init_sys` | 0 | 8KB | Once | WiFi + time sync |
| `tsk_init_azure` | 0 | 8KB | Once | Azure provisioning |
| `tsk_send_tels` | 0 | 8KB | 3s | Telemetry dispatcher |
| `tsk_az_loop` | 0 | 8KB | 3s | MQTT process loop |

### State Machine

#### Door Status
- `None` → `Closed` → `Opened` → `Closed` (cycle)

#### System Status
- `None` (Normal operation)
- `PasswordChangeMode` (Awaiting new password)
- `PasswordChanged` (Confirmation)
- `FingerprintEnrollmentMode` (Active enrollment)
- `RequestFingerprintEnrollmentMode` (Enrollment pending)

---

## 💤 Power Management

### Operating Modes

#### 🔆 Active Mode
- WiFi enabled for cloud connectivity
- Fingerprint reader: Power-gated (on-demand)
- Audio amplifier: Power-gated (on-demand)
- Motor driver: Power-gated (on-demand)
- Activity timer: 30-second countdown

#### 💤 Deep Sleep Mode
- **Trigger**: 30 seconds of inactivity
- **Power**: <10µA typical consumption
- **Wake Sources** (RTC GPIO):
  - 🎹 Any keypad key press (GPIO 11-14)
  - 🖐️ Fingerprint touch sensor (GPIO 9)
  - 🔘 Any button press (GPIO 8, 15, 16)
  - 👁️ PIR motion sensor (GPIO 0) *(optional)*

### Sleep Sequence
1. Set keypad columns HIGH
2. Enable GPIO wake-up (high-level trigger)
3. Enable fingerprint touch sensor
4. Hold all RTC GPIO states
5. Enter `esp_deep_sleep_start()`

### Wake-Up Flow
1. RTC GPIO interrupt triggers wake
2. Boot from deep sleep (retain RTC memory)
3. Re-initialize WiFi and Azure connection
4. Resume normal operation

---

## 🐛 Known Issues

### 1. ⚠️ Azure SDK Subscribe Functions
**Problem**: MQTT errors may occur when calling subscribe functions  
**Status**: Root cause unidentified  
**Note**: Cloud-to-device messaging not currently implemented

### 2. ⚠️ JM-101B Communication Hangs
**Problem**: UART deadlock if fingerprint reader disconnects during transmission  
**Status**: Known hardware limitation  
**Mitigation**: Cancellation token system partially handles this

### 3. ⚠️ Inaccurate Motor Control
**Problem**: Delay-based timing causes inconsistent lock/unlock  
**Recommendation**: Use hardware timer for precise motor control  
**Current**: `vTaskDelay(1000ms)` for motor operation

### 4. ⚠️ Light Sleep Not Implemented
**Problem**: Noticeable delay when waking from deep sleep  
**Solution**: Implement light sleep mode with fine-grained task control  
**Risk**: Requires careful task cancellation to prevent crashes

### 5. ⚠️ Memory Leak in CancellationTokenSource
**Problem**: Tokens created via `create_linked_token()` are never freed  
**Location**: `cancellationtokensource.cpp` line 38  
**Impact**: Memory leak (~48 bytes per token)

### 6. ⚠️ Incomplete RAII Implementation
**Problem**: Most resources use manual C-style management  
**Status**: Only UART and I2S use RAII destructors  
**Impact**: Semaphores and event groups may leak on error paths

---

## 🚧 TODO / Roadmap

- [ ] 🌙 **Light Sleep Mode** - Reduce wake-up latency with light sleep
- [ ] 🔄 **Executable Task Refactoring** - Fine-grained control for sleep modes
- [ ] 📱 **Mobile Notifications** - Integrate Azure Notification Hubs + Firebase
- [ ] ⏱️ **Precise Motor Control** - Hardware timer-based lock mechanism
- [ ] 🔍 **Debug Azure Subscribe** - Investigate MQTT subscription errors
- [ ] 🌐 **Multi-Language Audio** - Add English/Spanish/Chinese voice prompts
- [ ] 🔋 **Battery Monitoring** - Implement low-battery telemetry (Code 11)
- [ ] 🏠 **Home Assistant Integration** - MQTT bridge for smart home

---

## 📁 Project Structure

```
sls/
├── main/
│   ├── app_main.cpp                 # Main application logic
│   ├── config.h                     # System configuration
│   ├── azure/                       # Azure IoT integration
│   │   ├── dev_provisioning.cpp     # DPS registration
│   │   ├── iot_hub_provisioning.cpp # IoT Hub connection
│   │   ├── iot_hub_action.cpp       # Telemetry sender
│   │   └── network_helper.cpp       # TLS + SNTP
│   ├── fingerprint/                 # Biometric authentication
│   │   ├── reader.cpp               # JM-101B driver
│   │   └── helper.cpp               # Enrollment/search logic
│   ├── modules/                     # Hardware modules
│   │   ├── keypad.cpp               # Matrix keypad scanner
│   │   └── i2s_controller.cpp       # Audio playback
│   ├── audio/data/                  # Embedded audio files
│   │   └── metadata.cpp             # Audio registry
│   ├── helper/                      # Utilities
│   │   ├── system.cpp               # SNTP time sync
│   │   └── nvs.cpp                  # Storage operations
│   └── wifi/                        # Network connectivity
│       └── station.cpp              # WiFi manager
├── components/
│   └── azure-iot-esp32/             # ESP32 Azure SDK port
├── libs/
│   └── azure-iot-middleware-freertos/  # Azure SDK
└── CMakeLists.txt                   # Build configuration
```

---

## 🔧 Development

### Building from Source
```bash
# Set up ESP-IDF environment
. $HOME/esp/esp-idf/export.sh  # Linux/macOS
# OR
%USERPROFILE%\esp\esp-idf\export.bat  # Windows

# Configure project
idf.py menuconfig

# Build
idf.py build

# Flash + Monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

### Debugging
```bash
# Enable verbose logging in menuconfig
Component config → Log output → Verbose

# Monitor specific tags
idf.py monitor -p COM3 | grep "app_main"
```

### Code Style
- C++23 features enabled (gnu++2b)
- STL containers used alongside C-style arrays
- Limited RAII implementation (UART, I2S)
- Lambda-based task definitions
- Manual resource management for most peripherals

---

## 📜 License

This project is provided as-is for educational and development purposes.

**Academic Project** - Created as a university assignment by [@player-alex](https://github.com/player-alex)

---

## 🆘 Support

### Common Issues

**Q: Device won't connect to Azure**
A: Check WiFi credentials, verify SNTP time sync, confirm DPS credentials

**Q: Fingerprint reader not responding**
A: Check UART connections (GPIO 17/18), verify power transistor (GPIO 42)

**Q: Audio not playing**
A: Verify I2S connections, check power transistor (GPIO 40), test with `audio_data_beep.h`

**Q: Motor not working**
A: Check L298N connections (GPIO 38/39), verify power supply, test transistor (GPIO 7)

### Resources
- 📖 [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- ☁️ [Azure IoT Hub Docs](https://learn.microsoft.com/en-us/azure/iot-hub/)
- 🔧 [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

---

## 📝 Notes

⚠️ **This is not the final version** - Active development in progress

🎓 **Academic Project** - Created for university coursework

🔬 **Prototype Status** - Suitable for learning and experimentation

---

<div align="center">

**Built with ❤️ using ESP32-S3 and Azure IoT**

🎓 University Project by [@player-alex](https://github.com/player-alex)

🔐 Secure • ☁️ Connected • 🚀 Fast

[⬆ Back to Top](#-smart-door-lock-system)

</div>
