# 📐 System Schematic

Since we cannot include a Fritzing file here, this document describes the complete circuit in text and ASCII diagram format. Use this as a reference when wiring on your breadboard.

---

## 🗺️ System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    LoRa 433 MHz Broadcast Channel               │
│                   (All devices on same frequency)               │
└────────────┬───────────────────────────────────┬────────────────┘
             │                                   │
             ▼                                   ▼
   ┌──────────────────┐               ┌──────────────────┐
   │  FIREFIGHTER     │               │  FIREFIGHTER     │
   │    NODE 1        │               │    NODE 2        │
   │                  │               │                  │
   │  ESP32 WROOM-32  │               │  ESP32 WROOM-32  │
   │  LoRa Ra-02      │               │  LoRa Ra-02      │
   │  Pulse Sensor    │               │  Pulse Sensor    │
   │  OLED Display    │               │  OLED Display    │
   │  3 Buttons       │               │  3 Buttons       │
   │  Buzzer + LED    │               │  Buzzer + LED    │
   │  LiPo Battery    │               │  LiPo Battery    │
   └──────────────────┘               └──────────────────┘
             │                                   │
             └───────────────┬───────────────────┘
                             │
                             ▼
                  ┌──────────────────┐
                  │  CENTRAL SERVER  │
                  │                  │
                  │  ESP32 WROOM-32  │
                  │  LoRa Ra-02      │
                  │  Active Buzzer   │
                  │  USB Power       │
                  │                  │
                  │  WiFi Hotspot ──►│──► Smartphone Browser
                  │  FF_Network      │    http://192.168.4.1
                  └──────────────────┘
```

---

## 🔌 Node Circuit (ASCII Schematic)

```
                           3.3V Rail
                              │
              ┌───────────────┼──────────────────────┐
              │               │                      │
         LoRa VCC         OLED VCC           Pulse Sensor VCC
              │               │                      │
    ┌─────────┴──────┐  ┌─────┴──────┐    ┌─────────┴─────┐
    │  LoRa Ra-02    │  │OLED SSD1306│    │  Pulse Sensor  │
    │  SX1276        │  │  0x3C      │    │                │
    │                │  │            │    │                │
    │ NSS ───────────┼──┼────────────┼────┼── GPIO5        │
    │ RST ───────────┼──┼────────────┼────┼── GPIO14       │
    │ DIO0 ──────────┼──┼────────────┼────┼── GPIO2        │
    │ SCK ───────────┼──┼────────────┼────┼── GPIO18       │
    │ MISO ──────────┼──┼────────────┼────┼── GPIO19       │
    │ MOSI ──────────┼──┼────────────┼────┼── GPIO23       │
    │    SDA ────────┼──┼── GPIO21   │    │                │
    │    SCL ────────┼──┼── GPIO22   │    │ SIG ── GPIO34  │
    └────────────────┘  └────────────┘    └────────────────┘

              ESP32 WROOM-32
              ┌─────────────────────────────────────────┐
              │                                         │
  GPIO5  ─── NSS (LoRa CS)                             │
  GPIO14 ─── RST (LoRa Reset)                          │
  GPIO2  ─── DIO0 (LoRa IRQ)                           │
  GPIO18 ─── SCK  (SPI Clock)                          │
  GPIO19 ─── MISO (SPI Data In)                        │
  GPIO23 ─── MOSI (SPI Data Out)                       │
  GPIO21 ─── SDA  (OLED I2C Data)                      │
  GPIO22 ─── SCL  (OLED I2C Clock)                     │
  GPIO34 ─── Pulse Sensor SIG (ADC)                    │
  GPIO32 ─── BTN_HELP  (INPUT_PULLUP → GND)            │
  GPIO33 ─── BTN_EVACUATE (INPUT_PULLUP → GND)         │
  GPIO25 ─── BTN_SAFE  (INPUT_PULLUP → GND)            │
  GPIO26 ─── Buzzer (+)                                │
  GPIO27 ─── LED (+) via 220Ω                          │
  3.3V   ─── VCC rails for LoRa, OLED, Sensor          │
  GND    ─── GND rails for all components               │
  VIN    ─── TP4056 OUT+ (from LiPo)                   │
              └─────────────────────────────────────────┘


   Button Wiring (all same pattern):
   ┌──────────────┐
   │  Push Button │
   │  Pin1 ─────────── GPIO (32 / 33 / 25)
   │  Pin2 ─────────── GND
   └──────────────┘
   (INPUT_PULLUP: GPIO reads HIGH when open, LOW when pressed)


   Buzzer Wiring:
   GPIO26 ─── Active Buzzer (+) ─── Buzzer (-)  ─── GND


   LED Wiring:
   GPIO27 ─── 220Ω ─── LED Anode (+) ─── LED Cathode (-) ─── GND


   LiPo + TP4056 Wiring:
   LiPo (+) ─── TP4056 BAT+
   LiPo (-) ─── TP4056 BAT-
   TP4056 OUT+ ─── ESP32 VIN
   TP4056 OUT- ─── ESP32 GND
   (Charge via TP4056 USB port)
```

---

## 🖥️ Server Circuit (ASCII Schematic)

The server is simpler — LoRa + buzzer only.

```
              3.3V Rail
                  │
         ┌────────┘
         │
    LoRa VCC
         │
┌────────┴───────┐        ESP32 WROOM-32 (Server)
│  LoRa Ra-02    │        ┌──────────────────────────┐
│  SX1276        │        │                          │
│                │        │  GPIO5  ── NSS (LoRa CS) │
│ NSS ───────────┼────────┤  GPIO14 ── RST            │
│ RST ───────────┼────────┤  GPIO2  ── DIO0           │
│ DIO0 ──────────┼────────┤  GPIO18 ── SCK            │
│ SCK ───────────┼────────┤  GPIO19 ── MISO           │
│ MISO ──────────┼────────┤  GPIO23 ── MOSI           │
│ MOSI ──────────┼────────┤  GPIO26 ── Buzzer (+)     │
└────────────────┘        │  3.3V   ── LoRa VCC       │
                          │  GND    ── All GND         │
  Buzzer:                 │  USB    ── Power input     │
  GPIO26 ─── (+)          └──────────────────────────┘
  GND   ─── (-)
```

---

## 📡 LoRa Antenna

```
LoRa Module
┌─────────────┐
│             │
│  SMA Female │────── 433 MHz antenna
│  connector  │       (rubber duck or
└─────────────┘        flexible wire)
```

> Make sure the antenna is attached **before** powering the LoRa module.  
> Operating without antenna can damage the RF front end.

---

## ⚡ Power Architecture

```
BREADBOARD TESTING:
────────────────────────────────────────────────────────
  Smartphone Charger ──USB-C──► Node 1 ESP32 (5V → 3.3V via onboard regulator)
  Computer USB Port  ──USB──►  Node 2 ESP32 (5V → 3.3V via onboard regulator)
  USB Adapter        ──USB──►  Server ESP32
  
  ⚠️  GND rails between Node 1 and Node 2 breadboards
      must NEVER be connected to each other.

FINAL WEARABLE:
────────────────────────────────────────────────────────
  LiPo 1000mAh ──► TP4056 OUT ──► Node ESP32 VIN
  (Each node has its own battery — completely independent)
```

---

## 📏 Component Dimensions (for PCB planning)

| Component | Approx Size |
|---|---|
| ESP32 WROOM-32 Dev Board | 51.4 × 28.3 mm |
| LoRa Ra-02 Module | 17 × 16 mm |
| OLED SSD1306 | 27 × 27 mm (screen) |
| Analog Pulse Sensor | 16 mm diameter |
| TP4056 Module | 26 × 17 mm |
| LiPo 1000mAh (flat) | ~50 × 35 × 5 mm |

---

*Back to [README](../README.md)*
