# 🔥 Smart Emergency Communication & Safety Network for Firefighters

> **ESP32 + LoRa Ra-02 SX1276 @ 433 MHz** — Peer-to-peer emergency communication with real-time heart rate monitoring and a live IoT web dashboard. No mobile network. No internet. No infrastructure.

---

## 📸 Project Preview

```
╔══════════════════════════════════════╗
║   FIREFIGHTER NETWORK DASHBOARD      ║
║   ● LIVE    192.168.4.1              ║
╠══════════════════════════════════════╣
║  🚨 ALERT: NODE#1 NEEDS HELP         ║
╠══════════════╦═══════════════════════╣
║ FIREFIGHTER  ║  FIREFIGHTER          ║
║     #1       ║      #2               ║
║  85 BPM ●ON  ║   72 BPM ●ON         ║
║  CMD: HELP   ║  CMD: NONE            ║
╚══════════════╩═══════════════════════╝
```

---

## 🎯 What This Project Does

| Feature | Details |
|---|---|
| **Communication** | LoRa 433 MHz broadcast — all devices receive every packet simultaneously |
| **Range** | 200 metres indoor achieved on breadboard prototype |
| **Health Monitoring** | Analog pulse sensor reads heart rate continuously, included in every packet |
| **Emergency Buttons** | HELP / EVACUATE / SAFE — instant broadcast on press |
| **Peer Alerts** | OLED blinks × 3, buzzer sounds 5 sec, LED on — no server needed |
| **IoT Dashboard** | Hosted on ESP32 at `192.168.4.1` — no internet, no cloud, updates every 2 sec |
| **OFFLINE Detection** | Nodes not heard for 30 seconds are automatically marked OFFLINE |

---

## 👥 Team

| Name | Roll No |
|---|---|
| Joy Pastam | 23054-ES-002 |
| Adithya Chatla | 23054-ES-018 |
| K. Akhila | 23054-ES-043 |
| D. Chandana | 23054-ES-060 |
| P. Shiva Kumar | 23054-ES-062 |

**Guide:** LINGASWAMY SIR, Lecturer in ECE  
**Institute:** Govt. Institute of Electronics, East Marredpally, Secunderabad-500026  
**Year:** 2025-2026 | Diploma in Embedded Systems Engineering

---

## 📁 Repository Structure

```
firefighter-lora-esp32/
│
├── src/
│   ├── node1/
│   │   └── node1.ino          ← Firefighter Node 1 firmware
│   ├── node2/
│   │   └── node2.ino          ← Firefighter Node 2 firmware (DEVICE_ID=2)
│   └── server/
│       └── server.ino         ← Central server + IoT dashboard firmware
│
├── docs/
│   ├── WIRING.md              ← Complete pin-by-pin wiring guide
│   ├── SETUP.md               ← Arduino IDE setup instructions
│   └── TROUBLESHOOTING.md     ← Common problems and fixes
│
├── schematics/
│   └── SCHEMATIC.md           ← System schematic description
│
├── dashboard/
│   └── DASHBOARD.md           ← Dashboard usage guide
│
├── .gitignore
└── README.md                  ← This file
```

---

## 🛒 Hardware Required

### Per Firefighter Node (×2)

| Component | Specification | Supplier |
|---|---|---|
| ESP32 WROOM-32 | 240 MHz, 4MB Flash, WiFi | Robu.in |
| LoRa Ra-02 SX1276 | **433 MHz** ISM band | Robu.in |
| 433 MHz Antenna | SMA connector, flexible | Robu.in |
| Analog Pulse Sensor | PPG, 3.3V, SIG → ADC | Robu.in |
| OLED SSD1306 | 0.96", 128×64, I2C | Amazon India |
| Push Buttons | 2-pin momentary (×3) | Local store |
| Active Buzzer | 3.3V–5V DC, built-in oscillator | Amazon India |
| Red LED + 220Ω resistor | 5mm LED | Local store |
| LiPo Battery | 1000mAh, 3.7V | Amazon India |
| TP4056 Charger | USB charging + protection | Robu.in |

### Central Server (×1)

| Component | Notes |
|---|---|
| ESP32 WROOM-32 | Same as node |
| LoRa Ra-02 SX1276 (433 MHz) | Same as node |
| Active Buzzer | Alert at command center |
| USB Power Supply | 5V via USB |

> **⚠️ IMPORTANT:** Use **433 MHz** Ra-02 modules only. The 868 MHz and 915 MHz variants are NOT legal for use in India and will NOT communicate with these firmware files.

---

## ⚡ Quick Start

### Step 1 — Install Arduino IDE

Download [Arduino IDE 1.8.19](https://www.arduino.cc/en/software) and install it.

### Step 2 — Add ESP32 Board Support

1. Open `File → Preferences`
2. In **Additional Boards Manager URLs**, paste:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. Go to `Tools → Board → Boards Manager`
4. Search `esp32` and install **ESP32 by Espressif Systems**

### Step 3 — Install Libraries

Go to `Sketch → Include Library → Manage Libraries` and install:

| Library | Author |
|---|---|
| **LoRa** | Sandeep Mistry |
| **Adafruit SSD1306** | Adafruit (click "Install All" for dependencies) |
| **Adafruit GFX Library** | Adafruit |

### Step 4 — Select Board

`Tools → Board → ESP32 Arduino → ESP32 Dev Module`

### Step 5 — Upload Firmware

**Node 1:** Open `src/node1/node1.ino` → Upload  
**Node 2:** Open `src/node2/node2.ino` → Upload  
**Server:** Open `src/server/server.ino` → Upload

> **Upload Tip (Windows):** If you see *"Write timeout"* error, hold the **BOOT** button on the ESP32 while clicking Upload, release after you see the upload progress bar.

> **Linux Permission:** Run `sudo usermod -a -G dialout $USER` then log out and back in.

---

## 🔌 Wiring

### LoRa Module → ESP32 (same for all 3 devices)

| LoRa Pin | ESP32 GPIO | Notes |
|---|---|---|
| NSS | GPIO 5 | SPI Chip Select |
| RST | GPIO 14 | Hardware Reset |
| DIO0 | GPIO 2 | Packet Ready |
| SCK | GPIO 18 | SPI Clock |
| MISO | GPIO 19 | SPI Data In |
| MOSI | GPIO 23 | SPI Data Out |
| VCC | **3.3V only** | ⚠️ Never connect to 5V |
| GND (all 4 pins) | GND | Connect all GND pins |

### Node Components → ESP32

| Component | Pin | ESP32 GPIO |
|---|---|---|
| OLED SDA | SDA | GPIO 21 |
| OLED SCL | SCL | GPIO 22 |
| Pulse Sensor SIG | Signal | GPIO 34 (ADC only) |
| BTN HELP | One pin | GPIO 32 → GND |
| BTN EVACUATE | One pin | GPIO 33 → GND |
| BTN SAFE | One pin | GPIO 25 → GND |
| Buzzer (+) | Positive | GPIO 26 |
| LED (+) | Anode | GPIO 27 → 220Ω → LED |

---

## 📡 LoRa Radio Settings

All three devices use **identical** settings. Any mismatch = no communication.

```cpp
Frequency:       433E6     // 433 MHz
Spreading Factor: 9
Bandwidth:       125E3     // 125 kHz
Coding Rate:     5         // 4/5
TX Power:        17        // dBm
```

---

## 💬 Packet Protocol

| Type | Format | Example |
|---|---|---|
| Node Broadcast | `ID:<id>,HR:<bpm>,CMD:<cmd>,DST:ALL` | `ID:1,HR:85,CMD:HELP,DST:ALL` |
| Server ACK | `ACK:ID:<id>,STS:RECEIVED` | `ACK:ID:1,STS:RECEIVED` |
| Server Alert | `SRV:MSG:<message>` | `SRV:MSG:NODE#1 NEEDS HELP` |

---

## 🌐 Dashboard

1. Upload `server.ino` to the server ESP32
2. Connect your phone/laptop to WiFi: **`FF_Network`** (password: `fire1234`)
3. Open browser → **`http://192.168.4.1`**

| URL | Purpose |
|---|---|
| `http://192.168.4.1` | Live dashboard |
| `http://192.168.4.1/data` | Raw JSON data |
| `http://192.168.4.1/buzzer` | Silence server buzzer |

---

## 🔋 Power Supply During Testing

> **CRITICAL:** Use completely separate power sources for each node. Shared GND causes ground loop interference and erratic readings.

| Device | Power Source |
|---|---|
| Node 1 | Smartphone charger via USB Type-C |
| Node 2 | Computer USB port |
| Server | Laptop USB / USB adapter |

In the final wearable prototype, each node uses its own **1000mAh LiPo** battery with TP4056 charger (~3–4 hours operation).

---

## 🐛 Known Issues & Fixes

| Issue | Fix |
|---|---|
| Upload timeout ("Write timeout") | Hold BOOT button during "Connecting..." phase |
| Node 2 self-alert bug | Fixed: `startsWith("ID:")` check before peer processing |
| RF interference between nodes | Keep nodes ≥50 cm apart during breadboard testing |
| Ground loop — erratic HR readings | Fully isolate GND rails between breadboards |
| CH340 not detected (Windows) | Install CH340 USB driver from manufacturer |

---

## 📊 Test Results

| Parameter | Target | Achieved |
|---|---|---|
| Indoor range | 100 m | **200 m** ✅ |
| Button → peer alert | < 2 sec | **< 1 sec** ✅ |
| Dashboard refresh | < 5 sec | **2 sec** ✅ |
| OFFLINE detection | < 60 sec | **30 sec** ✅ |
| Mobile compatible | Yes | **Tested Android** ✅ |

---

## 🔮 Future Enhancements

- [ ] GPS location tracking (NEO-6M)
- [ ] Automatic fall detection (MPU6050 accelerometer)
- [ ] AES-128 packet encryption
- [ ] LoRa mesh networking for multi-floor buildings
- [ ] MAX30102 SpO2 + accurate HR sensor
- [ ] GSM SMS backup (SIM800L)
- [ ] SD card data logging

---

## 📜 License

This project is open source under the **MIT License** — free to use, modify, and distribute with attribution.

---

## ⭐ Give it a Star!

If this project helped you, please ⭐ star the repository on GitHub!  
It helps other students and developers find this project.

---

*Built with ❤️ by Team Embedded Systems 2025-2026 | Govt. Institute of Electronics, Secunderabad*
