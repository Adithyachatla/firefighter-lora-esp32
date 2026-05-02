# 🔌 Complete Wiring Guide

This document gives you the full pin-by-pin wiring for every device in the system.

---

## ⚠️ Critical Rules Before You Wire Anything

1. **LoRa VCC = 3.3V ONLY** — connecting it to 5V will permanently damage the module
2. **Connect all 4 GND pins** of the LoRa module to your GND rail
3. **GPIO34 is ADC-only** — do not try to use it as digital output
4. **Completely isolate GND rails** between Node 1 and Node 2 breadboards during testing

---

## 🔧 FIREFIGHTER NODE WIRING (Same for Node 1 and Node 2)

### LoRa Ra-02 SX1276 → ESP32

```
LoRa Module          ESP32 GPIO
───────────          ──────────
NSS  (CS)     →      GPIO 5
RST  (Reset)  →      GPIO 14
DIO0          →      GPIO 2
SCK           →      GPIO 18
MISO          →      GPIO 19
MOSI          →      GPIO 23
VCC           →      3.3V  ← ⚠️ 3.3V ONLY
GND (pin 1)   →      GND
GND (pin 2)   →      GND
GND (pin 3)   →      GND
GND (pin 4)   →      GND
```

> **DIO1 through DIO5** — leave these unconnected. The LoRa library used in this project only needs DIO0.

---

### OLED SSD1306 → ESP32

```
OLED Module          ESP32 GPIO
───────────          ──────────
SDA           →      GPIO 21
SCL           →      GPIO 22
VCC           →      3.3V
GND           →      GND
```

> I2C address: **0x3C**  
> The OLED shares the I2C bus but uses a different address from any other I2C devices.

---

### Analog Pulse Sensor → ESP32

```
Pulse Sensor         ESP32 GPIO
────────────         ──────────
SIG (Signal)  →      GPIO 34  ← ADC-only pin
VCC           →      3.3V
GND           →      GND
```

> Mount the sensor on the **underside** of the wrist device, with the sensor face touching skin.  
> GPIO34 has no internal pull-up and is input-only — perfect for analog reading.

---

### Push Buttons → ESP32

All buttons use `INPUT_PULLUP` mode — one pin goes to GPIO, other pin goes to GND.

```
Button           GPIO       GND
──────────────   ────────   ───
HELP   button →  GPIO 32  → GND
EVACUATE button→ GPIO 33  → GND
SAFE   button →  GPIO 25  → GND
```

> **Tip:** Use a red tactile button for HELP for easy identification under stress.  
> 300ms software debounce is already in the firmware.

---

### Active Buzzer → ESP32

```
Buzzer               ESP32 GPIO
──────               ──────────
Positive (+)  →      GPIO 26
Negative (-)  →      GND
```

> Use an **active** buzzer (has built-in oscillator, produces sound on DC voltage).  
> Do NOT use a passive buzzer — it needs a PWM frequency signal.

---

### Red LED → ESP32

```
LED + 220Ω Resistor  ESP32 GPIO
───────────────────  ──────────
Anode (+) → 220Ω →   GPIO 27
Cathode (-) →         GND
```

> The 220Ω resistor limits current to ~6mA — safe for ESP32 GPIO.

---

### LiPo Battery + TP4056 → ESP32

```
TP4056 Module        ESP32
─────────────        ─────
OUT+ (B+)     →      VIN pin  (or 5V pin)
OUT- (B-)     →      GND
```

> Charge the LiPo via the TP4056 Micro-USB/USB-C input port.  
> The TP4056 has built-in overcharge and over-discharge protection.

---

## 🖥️ CENTRAL SERVER WIRING

The server is simpler — no OLED, no buttons, no pulse sensor.

### LoRa Ra-02 SX1276 → ESP32 (same as node)

```
LoRa Module          ESP32 GPIO
───────────          ──────────
NSS           →      GPIO 5
RST           →      GPIO 14
DIO0          →      GPIO 2
SCK           →      GPIO 18
MISO          →      GPIO 19
MOSI          →      GPIO 23
VCC           →      3.3V
GND (all 4)   →      GND
```

### Active Buzzer → ESP32 (server)

```
Buzzer               ESP32 GPIO
──────               ──────────
Positive (+)  →      GPIO 26
Negative (-)  →      GND
```

Server is powered via **USB cable** from a laptop or USB adapter at the command post.

---

## 📐 Quick Reference Summary Table

| Component | Pin | ESP32 GPIO | Notes |
|---|---|---|---|
| LoRa NSS | NSS | 5 | SPI Chip Select |
| LoRa RST | RST | 14 | Hardware Reset |
| LoRa DIO0 | DIO0 | 2 | Packet Ready |
| LoRa SCK | SCK | 18 | SPI Clock |
| LoRa MISO | MISO | 19 | SPI Data In |
| LoRa MOSI | MOSI | 23 | SPI Data Out |
| LoRa VCC | VCC | **3.3V** | ⚠️ NOT 5V |
| LoRa GND | GND×4 | GND | All 4 pins |
| OLED SDA | SDA | 21 | I2C Data |
| OLED SCL | SCL | 22 | I2C Clock |
| Pulse Sensor | SIG | 34 | ADC only pin |
| BTN HELP | Pin1 | 32 | Other pin → GND |
| BTN EVACUATE | Pin1 | 33 | Other pin → GND |
| BTN SAFE | Pin1 | 25 | Other pin → GND |
| Buzzer (+) | + | 26 | Active buzzer |
| LED (+) | Anode | 27 | Via 220Ω |
| LiPo OUT+ | B+ | VIN | via TP4056 |

---

## 🧪 Testing Wiring Before Full Upload

Upload a simple blink sketch first to confirm ESP32 is working. Then test LoRa with the example sketches in the LoRa library (`File → Examples → LoRa → LoRaSender` and `LoRaReceiver`) before uploading the full firmware.

---

*Back to [README](../README.md)*
