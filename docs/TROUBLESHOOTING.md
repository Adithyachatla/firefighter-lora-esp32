# 🐛 Troubleshooting Guide

Every problem we actually encountered while building this project — and exactly how we fixed it.

---

## 🔴 Problem 1: Ground Loop Interference

### Symptoms
- Node 1 shows random resets while Node 2 is transmitting
- Pulse sensor reads wildly wrong values (30 BPM or 220 BPM with no finger contact)
- Serial Monitor shows garbage characters or resets
- One node works fine, the other behaves erratically

### Root Cause
When two breadboards share a GND rail through a common wire while powered from different USB sources, a tiny voltage difference between the two power supplies pushes current through the shared GND. This is called a **ground loop**.

### Fix ✅
1. **Remove any GND wire connecting the two breadboards**
2. Power Node 1 from a **smartphone charger** via USB
3. Power Node 2 from a **computer USB port**
4. Do NOT connect any wire between the two breadboards — not even GND

In the final wearable prototype, each node has its own LiPo battery so this is never an issue.

---

## 🔴 Problem 2: LoRa Modules Cannot Hear Each Other

### Symptoms
- Serial Monitor shows `[TX]` packets being sent but no `[RX]` on the other device
- Dashboard shows nodes OFFLINE even though they are powered on
- Packet reception works sometimes but not reliably

### Possible Causes and Fixes

#### Cause A: Antennas too close together
When LoRa modules are within 10 cm of each other, the transmitter signal overwhelms the receiver.

**Fix:** Separate the two breadboards by **at least 50 cm** during testing.

#### Cause B: Radio parameter mismatch
All three devices must have **identical** LoRa settings. Even one mismatch = zero communication.

**Fix:** Check these values are the same in all three `.ino` files:
```cpp
#define LORA_FREQ   433E6    // must be 433E6 for all
#define LORA_SF     9        // spreading factor
#define LORA_BW     125E3    // bandwidth
#define LORA_CR     5        // coding rate
```

#### Cause C: Using wrong frequency module
The 868 MHz and 915 MHz Ra-02 variants look identical but will not communicate with 433 MHz devices.

**Fix:** Check the module label. It should say **433** or **433M**.

#### Cause D: LoRa GND not fully connected
The Ra-02 has 4 GND pins. If any are floating, the module may transmit but not receive correctly.

**Fix:** Connect all 4 GND pins of the LoRa module to GND.

---

## 🔴 Problem 3: Node 2 Triggers Its Own Buzzer After Sending HELP

### Symptoms
- Node 2 buzzer and LED activate immediately after pressing HELP on Node 2
- It alerts itself instead of waiting for the peer to alert it

### Root Cause
The server ACK packet `ACK:ID:2,STS:RECEIVED` contains the substring `"ID:2"`. The original `extractInt()` function finds `"ID:"` inside the ACK packet and extracts `2`, which matches `DEVICE_ID = 2`, triggering a self-alert.

### Fix ✅
The firmware already includes this fix — a `startsWith("ID:")` guard:

```cpp
// ── Peer Node Packet ──
if (!inc.startsWith("ID:")) return;   // ← This line is the fix
```

This ensures only packets that **begin** with `"ID:"` are processed as peer broadcasts. ACK packets start with `"ACK:"` and are handled by the ACK branch before reaching this check.

If you are adapting this code and see self-alerts, make sure this line is present.

---

## 🔴 Problem 4: Upload Timeout on Windows

### Symptoms
- Arduino IDE shows: `A serial exception error occurred: Write timeout`
- Or: `Failed to connect to ESP32: Timed out... Connecting...`
- Upload gets stuck at the dots: `Connecting........_____`

### Fix ✅
1. Click the **Upload** button in Arduino IDE
2. Watch the bottom output area
3. When you see `Connecting....` appear
4. **Hold the BOOT button** on the ESP32 board
5. Keep holding until the upload progress bar starts moving
6. Release BOOT button

This manually puts the ESP32 into bootloader/download mode.

> Some ESP32 development boards have an automatic reset circuit and do not need this. But many cheap boards do require it.

---

## 🔴 Problem 5: CH340 Driver Not Installed (Windows)

### Symptoms
- ESP32 connected via USB but no COM port appears in Device Manager
- Device Manager shows "Unknown Device" with yellow warning icon
- Arduino IDE shows "No ports" under Tools → Port

### Fix ✅
1. Go to: https://www.wch-ic.com/downloads/CH341SER_EXE.html
2. Download and run the installer
3. Restart your computer
4. Reconnect ESP32 — the COM port should now appear

---

## 🔴 Problem 6: OLED Display Shows Nothing

### Symptoms
- OLED powers on (you can see a faint glow) but no text appears
- Serial Monitor shows `[ERROR] OLED init failed`

### Possible Causes

#### Cause A: Wrong I2C address
Most SSD1306 modules use `0x3C` but some use `0x3D`.

**Fix:** Change in code:
```cpp
#define OLED_ADDR  0x3D   // try this if 0x3C does not work
```

Or run an I2C scanner sketch to find the correct address.

#### Cause B: SDA/SCL wires swapped
**Fix:** Double-check OLED SDA → GPIO21 and OLED SCL → GPIO22.

#### Cause C: OLED powered from 5V
**Fix:** Connect OLED VCC to **3.3V**, not 5V.

---

## 🔴 Problem 7: Pulse Sensor Shows Wrong Heart Rate

### Symptoms
- Heart rate shows 50 BPM constantly with no finger
- Heart rate jumps between 50 and 180 randomly
- HR value does not change even with finger on sensor

### Possible Causes

#### Cause A: Sensor not connected to GPIO34
**Fix:** GPIO34 is the correct pin — it is ADC-only and works with analog sensors.

#### Cause B: Finger not making proper contact
**Fix:** Press finger firmly on the **green LED side** of the sensor. The green LED should illuminate your fingertip.

#### Cause C: Wrist mounting
The wrist has lower signal strength than fingertip. Normal — the sensor still works but the reading range is narrower. The `map()` range in the firmware accounts for this.

#### Cause D: Movement artifacts
Heart rate sensors show noise during movement. This is expected. The sensor works best when the arm is still.

---

## 🔴 Problem 8: Dashboard Does Not Load

### Symptoms
- Connected to FF_Network WiFi but browser shows "Site can't be reached"
- `http://192.168.4.1` does not open

### Possible Causes

#### Cause A: Server firmware not uploaded
Make sure you uploaded `server.ino` (not node firmware) to the server ESP32.

#### Cause B: Connected to wrong WiFi
Make sure your phone/laptop is connected to **FF_Network**, not your home WiFi.

#### Cause C: Using HTTPS instead of HTTP
**Fix:** Make sure the URL is `http://192.168.4.1` not `https://192.168.4.1`.

#### Cause D: Server ESP32 not powered
Check that the server ESP32 is connected and Serial Monitor shows `[OK] WiFi Hotspot: FF_Network`.

---

## 🔴 Problem 9: Library Not Found Error During Compilation

### Symptoms
- Error: `fatal error: LoRa.h: No such file or directory`
- Error: `fatal error: Adafruit_SSD1306.h: No such file or directory`

### Fix ✅
Install the missing library:

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for the missing library name
3. Click Install

| Missing Header | Library to Install |
|---|---|
| `LoRa.h` | LoRa by Sandeep Mistry |
| `Adafruit_SSD1306.h` | Adafruit SSD1306 |
| `Adafruit_GFX.h` | Adafruit GFX Library |

---

## 🔴 Problem 10: Nodes Show OFFLINE on Dashboard Immediately

### Symptoms
- Both nodes are powered on and transmitting but dashboard shows OFFLINE
- Serial Monitor on server shows `[RX]` packets arriving correctly

### Possible Cause
The `alertColor` field in `/data` resets to `green` after every dashboard poll. But the `online` status depends on `lastMs` timing.

**Fix:** Check that `OFFLINE_TIMEOUT` in `server.ino` is set correctly:
```cpp
#define OFFLINE_TIMEOUT  30000   // 30 seconds
```

If nodes transmit every 5 seconds, they should never go OFFLINE unless they actually stop transmitting.

---

## 📞 Still Stuck?

Open an issue on the GitHub repository with:
1. Which problem you are facing
2. What Serial Monitor shows on both devices
3. Which operating system you are using
4. Which exact ESP32 board you have

---

*Back to [README](../README.md)*
