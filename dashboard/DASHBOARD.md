# 🌐 IoT Web Dashboard Guide

The central server ESP32 hosts a live web dashboard that updates every 2 seconds. No internet, no cloud, no subscription required.

---

## 📱 How to Access the Dashboard

1. Upload `server.ino` to the server ESP32
2. Power the server ESP32 via USB
3. On your smartphone or laptop, go to **WiFi Settings**
4. Connect to the network: **`FF_Network`**
5. Password: **`fire1234`**
6. Open any browser and go to: **`http://192.168.4.1`**

> Use **`http://`** not `https://` — the ESP32 serves plain HTTP only.

---

## 🖥️ Dashboard Layout

```
┌────────────────────────────────────────────────────────────┐
│ 🔥  FIREFIGHTER NETWORK DASHBOARD              ● LIVE      │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  ✅ System Normal — All Firefighters Safe                  │  ← Alert Banner
│  (turns RED and flashes on HELP / EVACUATE)                │
│                                                            │
├───────────────────────┬────────────────────────────────────┤
│  FIREFIGHTER #1       │  FIREFIGHTER #2                    │
│  ● ONLINE             │  ● ONLINE                          │
│                       │                                    │
│  Heart Rate:          │  Heart Rate:                       │
│  85 BPM               │  72 BPM                            │
│  ████████░░ (bar)     │  ██████░░░░ (bar)                  │
│                       │                                    │
│  CMD: HELP            │  CMD: NONE                         │
│  Last seen: 0s ago    │  Last seen: 3s ago                 │
└───────────────────────┴────────────────────────────────────┘
│ Server: 192.168.4.1 | FF_Network | Silence: /buzzer | 14:32│
└────────────────────────────────────────────────────────────┘
```

---

## 🎨 Visual Indicators

### Alert Banner

| State | Colour | Text |
|---|---|---|
| Normal | 🟢 Dark Green | ✅ System Normal — All Firefighters Safe |
| Emergency | 🔴 Red (flashing) | 🚨 ALERT: NODE#1 NEEDS HELP |

### Node Status Dot

| Dot | Meaning |
|---|---|
| 🟢 Green | Node is ONLINE (packet received within 30 sec) |
| ⚫ Gray | Node is OFFLINE (no packet for 30+ sec) |

### Heart Rate Bar

| Bar Colour | Meaning |
|---|---|
| 🟢 Green | Heart rate is in normal range (≤100 BPM) |
| 🔴 Red | Heart rate is elevated (>100 BPM) — possible heat stress |

Bar width represents how close the HR is to the maximum range (50–180 BPM).

### CMD Badge Colours

| Badge | Colour |
|---|---|
| HELP | 🔴 Red |
| EVACUATE | 🟠 Orange |
| SAFE | 🟢 Green |
| NONE | ⚫ Gray |

---

## 🔗 API Endpoints

You can also access raw data if needed.

### `GET /` — Dashboard Page

Returns the full HTML dashboard page.

### `GET /data` — JSON Status

Returns current status of all firefighters as JSON.

**Example response:**
```json
{
  "alertColor": "red",
  "lastAlert": "NODE#1 NEEDS HELP",
  "ff": [
    {
      "id": 1,
      "hr": 85,
      "cmd": "HELP",
      "status": "ONLINE",
      "lastSeen": 2
    },
    {
      "id": 2,
      "hr": 72,
      "cmd": "NONE",
      "status": "ONLINE",
      "lastSeen": 5
    }
  ]
}
```

| Field | Type | Description |
|---|---|---|
| `alertColor` | string | `"red"` or `"green"` |
| `lastAlert` | string | Text of most recent alert |
| `ff[].id` | number | Firefighter node ID (1 or 2) |
| `ff[].hr` | number | Heart rate in BPM |
| `ff[].cmd` | string | Last command: HELP / EVACUATE / SAFE / NONE |
| `ff[].status` | string | `"ONLINE"` or `"OFFLINE"` |
| `ff[].lastSeen` | number | Seconds since last packet received |

### `GET /buzzer` — Silence Server Buzzer

Call this URL to immediately silence the server buzzer after acknowledging an alert.

```
http://192.168.4.1/buzzer
```

Returns plain text: `Buzzer silenced.`

---

## ⚙️ Dashboard Technical Details

| Property | Value |
|---|---|
| Hosted on | ESP32 flash memory |
| Server type | ESP32 WebServer library, port 80 |
| Update method | JavaScript `fetch('/data')` every 2000ms |
| Page size | ~4 KB (embedded in firmware) |
| Internet needed | ❌ None |
| Cloud needed | ❌ None |
| Max clients | Limited by ESP32 WiFi stack (~5 simultaneous) |
| Browser support | Any modern browser (Chrome, Firefox, Safari, Edge) |
| Mobile support | ✅ Responsive layout |

---

## 📶 WiFi Network Details

| Setting | Value |
|---|---|
| SSID (Network Name) | `FF_Network` |
| Password | `fire1234` |
| Server IP | `192.168.4.1` (fixed) |
| Client IP range | `192.168.4.2` to `192.168.4.254` |
| Protocol | HTTP (port 80) |
| Security | WPA2 |

> The WiFi hotspot is created by `WiFi.softAP()` — the ESP32 acts as a wireless router. No external router required.

---

## 🔧 Customising the Dashboard

The dashboard HTML is embedded as a string in `server.ino` inside `DASHBOARD_HTML[]`.

To change colours, layout, or add features:

1. Open `src/server/server.ino`
2. Find the section:
   ```cpp
   const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
   <!DOCTYPE html>
   ...
   )rawliteral";
   ```
3. Edit the HTML/CSS/JavaScript inside
4. Save and re-upload to the server ESP32

---

*Back to [README](../README.md)*
