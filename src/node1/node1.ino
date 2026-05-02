/*
 * ============================================================
 *  SMART EMERGENCY COMMUNICATION & SAFETY NETWORK
 *  FOR FIREFIGHTERS — NODE 1 FIRMWARE
 * ============================================================
 *  Hardware : ESP32 WROOM-32 + LoRa Ra-02 SX1276 @ 433 MHz
 *  Sensors  : Analog Pulse Sensor (GPIO34), 3x Push Buttons
 *  Output   : OLED SSD1306, Active Buzzer, Red LED
 *  Power    : 1000mAh LiPo + TP4056 charger
 *
 *  Team     : Joy Pastam · Adithya Chatla · K. Akhila
 *             D. Chandana · P. Shiva Kumar
 *  Guide    : LINGASWAMY SIR, Lecturer in ECE
 *  Institute: Govt. Institute of Electronics, Secunderabad
 *  Year     : 2025-2026
 * ============================================================
 *
 *  NOTE: For Node 2, change:
 *    #define DEVICE_ID  2
 *    #define PEER_ID    1
 *  Everything else stays the same.
 *
 *  Packet formats:
 *    TX : ID:<id>,HR:<bpm>,CMD:<cmd>,DST:ALL
 *    RX  ACK: ACK:ID:<id>,STS:RECEIVED
 *    RX  SRV: SRV:MSG:<alert text>
 *    RX  Peer: ID:<id>,HR:<bpm>,CMD:<cmd>,DST:ALL
 * ============================================================
 */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── Device Identity ──────────────────────────────────────
#define DEVICE_ID     1          // Change to 2 for Node 2
#define PEER_ID       2          // Change to 1 for Node 2

// ── LoRa SPI Pin Mapping ─────────────────────────────────
#define LORA_NSS      5
#define LORA_RST      14
#define LORA_DIO0     2
#define LORA_FREQ     433E6      // 433 MHz — India ISM band

// ── LoRa Radio Parameters (ALL devices must match) ───────
#define LORA_SF       9
#define LORA_BW       125E3
#define LORA_CR       5          // Coding rate 4/5
#define LORA_TX_POW   17         // dBm

// ── OLED Configuration ───────────────────────────────────
#define OLED_WIDTH    128
#define OLED_HEIGHT   64
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// ── GPIO Pins ────────────────────────────────────────────
#define BTN_HELP      32         // HELP button
#define BTN_EVACUATE  33         // EVACUATE button
#define BTN_SAFE      25         // SAFE button
#define BUZZER_PIN    26         // Active buzzer
#define LED_PIN       27         // Red LED (via 220Ω resistor)
#define PULSE_PIN     34         // Analog pulse sensor (ADC only)

// ── Timing Constants ─────────────────────────────────────
#define SEND_INTERVAL    5000    // ms — periodic heartbeat
#define DEBOUNCE_MS      300     // ms — button debounce
#define ALERT_DURATION   5000   // ms — buzzer/LED on time
#define PULSE_SAMPLE_MS  20      // ms — sensor sampling rate

// ── State Variables ──────────────────────────────────────
String myCmd       = "NONE";
int    myHR        = 72;         // default until sensor stabilises
int    peerHR      = 0;
String peerCmd     = "---";
String lastMsg     = "System Ready";

unsigned long lastSendMs    = 0;
unsigned long lastBtnHelpMs = 0;
unsigned long lastBtnEvacMs = 0;
unsigned long lastBtnSafeMs = 0;
unsigned long lastPulseMs   = 0;
unsigned long alertUntilMs  = 0;

// ── Setup ────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n[BOOT] Firefighter Node #" + String(DEVICE_ID)));

  // GPIO
  pinMode(BTN_HELP,     INPUT_PULLUP);
  pinMode(BTN_EVACUATE, INPUT_PULLUP);
  pinMode(BTN_SAFE,     INPUT_PULLUP);
  pinMode(BUZZER_PIN,   OUTPUT);
  pinMode(LED_PIN,      OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN,    LOW);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[ERROR] OLED init failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("FF Node #" + String(DEVICE_ID));
  display.println("Initialising LoRa...");
  display.display();

  // LoRa
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println(F("[ERROR] LoRa init failed — check wiring"));
    display.println("LoRa FAILED!");
    display.display();
    while (true);
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CR);
  LoRa.setTxPower(LORA_TX_POW);
  LoRa.receive();

  Serial.println(F("[OK] LoRa ready @ 433 MHz"));
  Serial.println(F("[OK] Node Ready"));
  updateOLED();
}

// ── Main Loop ────────────────────────────────────────────
void loop() {
  readPulse();
  checkButtons();
  periodicSend();
  receiveLoRa();
  checkAlertTimer();
}

// ── Pulse Sensor ─────────────────────────────────────────
void readPulse() {
  if (millis() - lastPulseMs < PULSE_SAMPLE_MS) return;
  lastPulseMs = millis();

  int raw = analogRead(PULSE_PIN);
  if (raw > 2000) {                         // finger/wrist contact detected
    myHR = constrain(map(raw, 2000, 4095, 60, 110), 50, 180);
  }
}

// ── Button Handling ──────────────────────────────────────
void checkButtons() {
  unsigned long now = millis();

  if (digitalRead(BTN_HELP) == LOW && now - lastBtnHelpMs > DEBOUNCE_MS) {
    lastBtnHelpMs = now;
    myCmd = "HELP";
    Serial.println(F("[BTN] HELP pressed"));
    sendPacket();
  }
  if (digitalRead(BTN_EVACUATE) == LOW && now - lastBtnEvacMs > DEBOUNCE_MS) {
    lastBtnEvacMs = now;
    myCmd = "EVACUATE";
    Serial.println(F("[BTN] EVACUATE pressed"));
    sendPacket();
  }
  if (digitalRead(BTN_SAFE) == LOW && now - lastBtnSafeMs > DEBOUNCE_MS) {
    lastBtnSafeMs = now;
    myCmd = "SAFE";
    Serial.println(F("[BTN] SAFE pressed"));
    sendPacket();
  }
}

// ── Periodic Heartbeat ───────────────────────────────────
void periodicSend() {
  if (millis() - lastSendMs < SEND_INTERVAL) return;
  lastSendMs = millis();
  sendPacket();
}

// ── Transmit Packet ──────────────────────────────────────
void sendPacket() {
  String pkt = "ID:"  + String(DEVICE_ID) +
               ",HR:" + String(myHR) +
               ",CMD:" + myCmd +
               ",DST:ALL";

  LoRa.beginPacket();
  LoRa.print(pkt);
  LoRa.endPacket();
  LoRa.receive();

  Serial.println("[TX] " + pkt);
  myCmd = "NONE";
  updateOLED();
}

// ── Receive & Route Packets ──────────────────────────────
void receiveLoRa() {
  int sz = LoRa.parsePacket();
  if (sz == 0) return;

  String inc = "";
  while (LoRa.available()) inc += (char)LoRa.read();
  int rssi = LoRa.packetRssi();
  Serial.println("[RX] " + inc + " RSSI=" + String(rssi));

  // ── Server ACK ──
  if (inc.startsWith("ACK:")) {
    int ackId = extractInt(inc, "ID:", ",");
    if (ackId == DEVICE_ID) {
      lastMsg = "Server: ACK OK";
      updateOLED();
    }
    return;
  }

  // ── Server Emergency Broadcast ──
  if (inc.startsWith("SRV:MSG:")) {
    String msg = inc.substring(8);
    lastMsg = msg;
    Serial.println("[SRV] " + msg);
    triggerAlert(ALERT_DURATION);
    return;
  }

  // ── Peer Node Packet ──
  if (!inc.startsWith("ID:")) return;

  int sid = extractInt(inc, "ID:", ",");
  if (sid == DEVICE_ID) return;            // own echo — ignore

  int  hr  = extractInt(inc, "HR:", ",");
  String cmd = extractField(inc, "CMD:", ",");

  peerHR  = hr;
  peerCmd = cmd;

  if (cmd == "HELP" || cmd == "EVACUATE") {
    lastMsg = "!! N" + String(sid) + ": " + cmd + " !!";
    triggerAlert(ALERT_DURATION);
  } else {
    if (cmd == "SAFE") lastMsg = "N" + String(sid) + " is SAFE";
    updateOLED();
  }
}

// ── OLED Display ─────────────────────────────────────────
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("=== NODE #" + String(DEVICE_ID) + " ===");

  display.setCursor(0, 12);
  display.println("Me  HR: " + String(myHR) + " bpm");

  display.setCursor(0, 22);
  display.println("N" + String(PEER_ID) + "  HR: " +
                  (peerHR > 0 ? String(peerHR) + " bpm" : "---"));

  display.drawLine(0, 32, 127, 32, SSD1306_WHITE);

  display.setCursor(0, 35);
  display.println("N" + String(PEER_ID) + " CMD: " + peerCmd);

  display.setCursor(0, 46);
  display.println(lastMsg.substring(0, 21));

  display.setCursor(0, 56);
  display.println("MeCmd: " + myCmd);

  display.display();
}

// ── Alert: Buzzer + LED + OLED Blink ────────────────────
void triggerAlert(unsigned long durationMs) {
  alertUntilMs = millis() + durationMs;
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN,    HIGH);

  // OLED blink × 3
  for (int i = 0; i < 3; i++) {
    display.invertDisplay(true);  delay(150);
    display.invertDisplay(false); delay(150);
  }
  updateOLED();
}

void checkAlertTimer() {
  if (alertUntilMs > 0 && millis() >= alertUntilMs) {
    alertUntilMs = 0;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN,    LOW);
  }
}

// ── String Helpers ───────────────────────────────────────
int extractInt(String s, String key, String delim) {
  int start = s.indexOf(key);
  if (start < 0) return -1;
  start += key.length();
  int end = s.indexOf(delim, start);
  if (end < 0) end = s.length();
  return s.substring(start, end).toInt();
}

String extractField(String s, String key, String delim) {
  int start = s.indexOf(key);
  if (start < 0) return "---";
  start += key.length();
  int end = s.indexOf(delim, start);
  if (end < 0) end = s.length();
  return s.substring(start, end);
}
