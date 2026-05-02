# ⚙️ Arduino IDE Setup Guide

Complete step-by-step instructions to set up your development environment and upload firmware to all three ESP32 devices.

---

## 📥 Step 1 — Download Arduino IDE

Go to **https://www.arduino.cc/en/software** and download **Arduino IDE 1.8.19**.

| OS | Download |
|---|---|
| Windows | Arduino-1.8.19-windows.exe |
| Ubuntu / Linux | arduino-1.8.19-linux64.tar.xz |
| macOS | arduino-1.8.19-macosx.zip |

Install and open it.

---

## 🔧 Step 2 — Add ESP32 Board Support

The Arduino IDE does not support ESP32 by default. You need to add it.

1. Open **File → Preferences**
2. Find the field: **Additional Boards Manager URLs**
3. Paste this URL:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Click **OK**

Now install the board package:

1. Go to **Tools → Board → Boards Manager**
2. In the search box type: `esp32`
3. Find **ESP32 by Espressif Systems**
4. Click **Install** (this may take a few minutes)
5. Wait until it says "Installed"

---

## 📋 Step 3 — Select the Right Board

1. Go to **Tools → Board → ESP32 Arduino**
2. Select **ESP32 Dev Module**

Set these settings under Tools:

| Setting | Value |
|---|---|
| Board | ESP32 Dev Module |
| Upload Speed | 921600 |
| CPU Frequency | 240 MHz |
| Flash Frequency | 80 MHz |
| Flash Mode | QIO |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Default 4MB with spiffs |
| Core Debug Level | None |
| Port | (your COM port — see Step 5) |

---

## 📚 Step 4 — Install Required Libraries

Go to **Sketch → Include Library → Manage Libraries**

Search and install each of these:

### Library 1: LoRa

- Search: `LoRa`
- Install: **LoRa** by **Sandeep Mistry**

### Library 2: Adafruit SSD1306

- Search: `Adafruit SSD1306`
- Install: **Adafruit SSD1306** by **Adafruit**
- When a pop-up appears asking about dependencies → click **Install All**
- This will also install **Adafruit GFX Library** automatically

### Built-in Libraries (no install needed)

These come with the ESP32 board package:

| Library | Used For |
|---|---|
| `SPI.h` | LoRa SPI communication |
| `Wire.h` | OLED I2C communication |
| `WiFi.h` | Server WiFi hotspot |
| `WebServer.h` | HTTP server for dashboard |

---

## 🔌 Step 5 — Connect ESP32 and Find the Port

### Windows

1. Connect ESP32 via USB cable
2. Open **Device Manager** (Win+X → Device Manager)
3. Look under **Ports (COM & LPT)**
4. You should see something like **USB-SERIAL CH340 (COM5)**
5. Note the COM number (e.g., COM5)
6. In Arduino IDE: **Tools → Port → COM5**

> **If you don't see any COM port:**  
> Install the CH340 driver from: https://www.wch-ic.com/downloads/CH341SER_EXE.html  
> Restart your computer after installing.

### Ubuntu / Linux

1. Connect ESP32 via USB
2. Open terminal and run:
   ```bash
   ls /dev/tty*
   ```
3. You should see `/dev/ttyUSB0` or `/dev/ttyACM0`
4. In Arduino IDE: **Tools → Port → /dev/ttyUSB0**

> **Permission denied error on Linux?**  
> Run this command in terminal:
> ```bash
> sudo usermod -a -G dialout $USER
> ```
> Then **log out and log back in** (not just close terminal — full logout).  
> This only needs to be done once.

### macOS

1. Connect ESP32 via USB
2. In Arduino IDE: **Tools → Port**
3. Select the port that shows `/dev/cu.SLAB_USBtoUART` or similar

---

## 📤 Step 6 — Upload Firmware

### Upload to Node 1

1. Open `src/node1/node1.ino` in Arduino IDE
2. Confirm `DEVICE_ID = 1` and `PEER_ID = 2` in the code
3. Select correct **Board** and **Port**
4. Click the **Upload** button (→ arrow icon)
5. Watch the progress bar at the bottom

### Upload to Node 2

1. Open `src/node2/node2.ino`
2. Confirm `DEVICE_ID = 2` and `PEER_ID = 1` in the code
3. Select correct **Port** (reconnect this ESP32)
4. Click **Upload**

### Upload to Server

1. Open `src/server/server.ino`
2. Click **Upload**

---

## ⚠️ Upload Troubleshooting

### Error: "Write timeout" or "Failed to connect to ESP32"

This is the most common issue on Windows.

**Fix:**
1. Click the **Upload** button in Arduino IDE
2. Watch the bottom of the IDE — wait until you see `Connecting....`
3. At that exact moment, **press and hold the BOOT button** on the ESP32 board
4. Hold it until you see the upload progress bar moving
5. Release the BOOT button

### Error: "Permission denied" on Linux

```bash
sudo usermod -a -G dialout $USER
```
Log out completely and log back in.

### Error: "COM port not found" on Windows

Install CH340 driver:  
https://www.wch-ic.com/downloads/CH341SER_EXE.html

### Error: "Sketch too large"

Go to **Tools → Partition Scheme → Huge APP (3MB No OTA)**

### Upload completes but nothing works

- Open **Tools → Serial Monitor**
- Set baud rate to **115200**
- Press the **EN/Reset** button on ESP32
- You should see boot messages like `[OK] LoRa ready @ 433 MHz`

---

## ✅ Step 7 — Verify Everything Works

After uploading to all three devices:

1. **Open Serial Monitor** (`Tools → Serial Monitor`) at **115200 baud**

2. For Node 1 you should see:
   ```
   [BOOT] Firefighter Node #1
   [OK] LoRa ready @ 433 MHz
   [OK] Node Ready
   [TX] ID:1,HR:72,CMD:NONE,DST:ALL
   ```

3. For Server you should see:
   ```
   [BOOT] Firefighter Server
   [OK] LoRa ready @ 433 MHz
   [OK] WiFi Hotspot: FF_Network
   [OK] Dashboard at: http://192.168.4.1
   [OK] Server ready. Waiting for nodes...
   ```

4. After a few seconds the server should show:
   ```
   [RX] ID:1,HR:72,CMD:NONE,DST:ALL RSSI=-65
   [TX ACK] ACK:ID:1,STS:RECEIVED
   ```

5. Connect your phone to **FF_Network** (password: `fire1234`)

6. Open browser → `http://192.168.4.1`

7. Dashboard should show both nodes as **ONLINE** ✅

---

## 🔁 Re-uploading After Changes

If you change anything in the code:

1. Make your edits
2. Save the file (`Ctrl+S`)
3. Click Upload
4. Wait for upload to complete
5. The ESP32 restarts automatically

You do **not** need to unplug and replug between uploads.

---

*Back to [README](../README.md)*
