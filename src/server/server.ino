/*
 * ============================================================
 *  SMART EMERGENCY COMMUNICATION & SAFETY NETWORK
 *  FOR FIREFIGHTERS — CENTRAL SERVER FIRMWARE
 * ============================================================
 *  Hardware : ESP32 WROOM-32 + LoRa Ra-02 SX1276 @ 433 MHz
 *  Output   : Active Buzzer (GPIO26)
 *  Network  : WiFi Access Point  SSID: FF_Network  PW: fire1234
 *  Dashboard: http://192.168.4.1
 *
 *  Team     : Joy Pastam · Adithya Chatla · K. Akhila
 *             D. Chandana · P. Shiva Kumar
 *  Guide    : LINGASWAMY SIR, Lecturer in ECE
 *  Institute: Govt. Institute of Electronics, Secunderabad
 *  Year     : 2025-2026
 * ============================================================
 *
 *  API Endpoints:
 *    GET /       — HTML dashboard page
 *    GET /data   — JSON with both firefighter statuses
 *    GET /buzzer — Silences server buzzer remotely
 *
 *  JSON Response (/data):
 *  {
 *    "alertColor": "red" | "green",
 *    "lastAlert":  "NODE#1 NEEDS HELP",
 *    "ff": [
 *      { "id":1, "hr":85, "cmd":"HELP",
 *        "status":"ONLINE", "lastSeen":2 },
 *      { "id":2, "hr":72, "cmd":"NONE",
 *        "status":"ONLINE", "lastSeen":5 }
 *    ]
 *  }
 * ============================================================
 */

#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>

// ── WiFi Access Point ────────────────────────────────────
const char* AP_SSID     = "FF_Network";
const char* AP_PASSWORD = "fire1234";

// ── LoRa SPI Pin Mapping ─────────────────────────────────
#define LORA_NSS    5
#define LORA_RST    14
#define LORA_DIO0   2
#define LORA_FREQ   433E6

// ── LoRa Radio Parameters (must match all nodes) ─────────
#define LORA_SF     9
#define LORA_BW     125E3
#define LORA_CR     5
#define LORA_TX_POW 17

// ── GPIO ──────────────────────────────────────────────────
#define BUZZER_PIN  26

// ── Timing ───────────────────────────────────────────────
#define OFFLINE_TIMEOUT  30000   // ms — node marked offline after this
#define BUZZER_DURATION  5000    // ms — server buzzer on time

// ── Firefighter Status Record ─────────────────────────────
struct FirefighterRecord {
  int      id      = 0;
  int      hr      = 0;
  String   cmd     = "---";
  bool     online  = false;
  unsigned long lastMs = 0;
};

FirefighterRecord ff[2];         // ff[0] = Node1, ff[1] = Node2

// ── Alert State ───────────────────────────────────────────
String       lastAlert  = "No alerts";
String       alertColor = "green";
unsigned long buzzerStop = 0;

// ── Web Server ────────────────────────────────────────────
WebServer server(80);

// ════════════════════════════════════════════════════════════
// HTML DASHBOARD (single-page, embedded in flash)
// ════════════════════════════════════════════════════════════
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Firefighter Network Dashboard</title>
<style>
  * { box-sizing:border-box; margin:0; padding:0; }
  body { font-family:'Segoe UI',sans-serif; background:#0a1929; color:#fff; min-height:100vh; }
  header { background:#0d47a1; padding:14px 20px; display:flex; align-items:center; gap:12px; }
  header h1 { font-size:18px; font-weight:700; letter-spacing:1px; }
  .live { background:#1db954; color:#fff; font-size:10px; padding:3px 8px; border-radius:12px; font-weight:700; animation:pulse 1.5s infinite; }
  @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:.5} }
  #alert-banner { padding:12px 20px; text-align:center; font-weight:700; font-size:15px; transition:background .4s; }
  .alert-green { background:#1b5e20; }
  .alert-red   { background:#b71c1c; animation:flashRed 1s infinite; }
  @keyframes flashRed { 0%,100%{opacity:1} 50%{opacity:.7} }
  .cards { display:flex; gap:16px; padding:16px; flex-wrap:wrap; }
  .card { flex:1; min-width:260px; background:#0d2137; border-radius:12px; padding:18px; border:2px solid #1565c0; }
  .card.danger { border-color:#f44336; }
  .card-header { display:flex; justify-content:space-between; align-items:center; margin-bottom:12px; }
  .card-title { font-size:14px; font-weight:700; }
  .dot { width:10px; height:10px; border-radius:50%; display:inline-block; margin-right:6px; }
  .dot-green { background:#1db954; }
  .dot-red   { background:#f44336; }
  .dot-gray  { background:#666; }
  .hr-val { font-size:32px; font-weight:900; margin:8px 0 4px; }
  .hr-bar-bg { background:#1a3a5c; border-radius:8px; height:10px; margin:8px 0; }
  .hr-bar { height:10px; border-radius:8px; transition:width .5s, background .5s; }
  .cmd-badge { display:inline-block; padding:4px 14px; border-radius:20px; font-size:12px; font-weight:700; margin-top:6px; }
  .cmd-HELP     { background:#f44336; }
  .cmd-EVACUATE { background:#ff9800; color:#000; }
  .cmd-SAFE     { background:#1db954; }
  .cmd-NONE     { background:#455a64; }
  .cmd-default  { background:#455a64; }
  .last-seen { font-size:11px; color:#78909c; margin-top:8px; }
  footer { text-align:center; padding:10px; font-size:11px; color:#546e7a; border-top:1px solid #1a3a5c; margin-top:8px; }
</style>
</head>
<body>
<header>
  <span style="font-size:22px">🔥</span>
  <h1>FIREFIGHTER NETWORK DASHBOARD</h1>
  <span class="live">● LIVE</span>
</header>

<div id="alert-banner" class="alert-green">✅ System Normal — All Firefighters Safe</div>

<div class="cards" id="cards-container">
  <div class="card" id="card-1">
    <div class="card-header">
      <span class="card-title"><span class="dot dot-gray" id="dot-1"></span>FIREFIGHTER #1</span>
      <span id="status-1" style="font-size:11px;color:#78909c">---</span>
    </div>
    <div>Heart Rate:</div>
    <div class="hr-val" id="hr-1">-- BPM</div>
    <div class="hr-bar-bg"><div class="hr-bar" id="bar-1" style="width:0%;background:#1db954"></div></div>
    <div id="cmd-1" class="cmd-badge cmd-default">---</div>
    <div class="last-seen" id="seen-1">Waiting for data...</div>
  </div>
  <div class="card" id="card-2">
    <div class="card-header">
      <span class="card-title"><span class="dot dot-gray" id="dot-2"></span>FIREFIGHTER #2</span>
      <span id="status-2" style="font-size:11px;color:#78909c">---</span>
    </div>
    <div>Heart Rate:</div>
    <div class="hr-val" id="hr-2">-- BPM</div>
    <div class="hr-bar-bg"><div class="hr-bar" id="bar-2" style="width:0%;background:#1db954"></div></div>
    <div id="cmd-2" class="cmd-badge cmd-default">---</div>
    <div class="last-seen" id="seen-2">Waiting for data...</div>
  </div>
</div>

<footer>
  Server IP: 192.168.4.1 &nbsp;|&nbsp; Network: FF_Network &nbsp;|&nbsp;
  Silence buzzer: <a href="/buzzer" style="color:#4fc3f7">192.168.4.1/buzzer</a> &nbsp;|&nbsp;
  <span id="ts"></span>
</footer>

<script>
function update() {
  fetch('/data').then(r=>r.json()).then(d=>{
    // Alert banner
    var ab = document.getElementById('alert-banner');
    ab.className = d.alertColor==='red' ? 'alert-red' : 'alert-green';
    ab.textContent = d.alertColor==='red' ? '🚨 ALERT: ' + d.lastAlert : '✅ System Normal — All Firefighters Safe';

    // Each firefighter card
    d.ff.forEach(function(f){
      var i = f.id;
      var online = f.status==='ONLINE';

      // Dot & status
      var dot = document.getElementById('dot-'+i);
      dot.className = 'dot ' + (online ? 'dot-green' : 'dot-gray');
      document.getElementById('status-'+i).textContent = f.status;

      // HR
      document.getElementById('hr-'+i).textContent = online ? f.hr+' BPM' : '-- BPM';

      // Bar
      var pct = Math.min(100, Math.max(0, ((f.hr-50)/130)*100));
      var bar = document.getElementById('bar-'+i);
      bar.style.width = (online ? pct : 0) + '%';
      bar.style.background = f.hr>100 ? '#f44336' : '#1db954';

      // CMD badge
      var cb = document.getElementById('cmd-'+i);
      var cls = ['HELP','EVACUATE','SAFE','NONE'].includes(f.cmd) ? 'cmd-'+f.cmd : 'cmd-default';
      cb.className = 'cmd-badge ' + cls;
      cb.textContent = 'CMD: ' + (online ? f.cmd : '---');

      // Card border
      document.getElementById('card-'+i).className = 'card' + (f.cmd==='HELP'||f.cmd==='EVACUATE' ? ' danger' : '');

      // Last seen
      document.getElementById('seen-'+i).textContent = online
        ? 'Last seen: ' + f.lastSeen + 's ago'
        : 'OFFLINE — no signal for ' + f.lastSeen + 's';
    });

    document.getElementById('ts').textContent = new Date().toLocaleTimeString();
  }).catch(function(){ });
}
setInterval(update, 2000);
update();
</script>
</body>
</html>
)rawliteral";

// ════════════════════════════════════════════════════════════
// SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n[BOOT] Firefighter Server"));

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialise records
  ff[0].id = 1;
  ff[1].id = 2;

  // LoRa
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println(F("[ERROR] LoRa init failed"));
    while (true);
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CR);
  LoRa.setTxPower(LORA_TX_POW);
  LoRa.receive();
  Serial.println(F("[OK] LoRa ready @ 433 MHz"));

  // WiFi Access Point
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("[OK] WiFi Hotspot: " + String(AP_SSID));
  Serial.println("[OK] Dashboard at: http://" + WiFi.softAPIP().toString());

  // HTTP routes
  server.on("/",       handleDashboard);
  server.on("/data",   handleData);
  server.on("/buzzer", handleBuzzerSilence);
  server.begin();
  Serial.println(F("[OK] Server ready. Waiting for nodes..."));
}

// ════════════════════════════════════════════════════════════
// MAIN LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  server.handleClient();
  receiveAndProcess();
  checkOfflineStatus();
  checkBuzzerTimer();
}

// ── Receive Packet and Process ───────────────────────────
void receiveAndProcess() {
  int sz = LoRa.parsePacket();
  if (sz == 0) return;

  String pkt = "";
  while (LoRa.available()) pkt += (char)LoRa.read();
  int rssi = LoRa.packetRssi();
  Serial.println("[RX] " + pkt + " RSSI=" + String(rssi));

  if (!pkt.startsWith("ID:")) return;

  int    id  = extractInt(pkt, "ID:", ",");
  int    hr  = extractInt(pkt, "HR:", ",");
  String cmd = extractField(pkt, "CMD:", ",");

  if (id < 1 || id > 2) return;
  updateFF(id, hr, cmd);
  sendAck(id);

  if (cmd == "HELP" || cmd == "EVACUATE") {
    lastAlert  = "NODE#" + String(id) + " NEEDS " + cmd;
    alertColor = "red";
    String bcast = "SRV:MSG:NODE#" + String(id) + " NEEDS " + cmd;
    broadcastAll(bcast);
    activateBuzzer();
    Serial.println("[ALERT] " + lastAlert);
  }
}

// ── Update Firefighter Record ─────────────────────────────
void updateFF(int id, int hr, String cmd) {
  int idx = id - 1;
  ff[idx].id     = id;
  ff[idx].hr     = hr;
  ff[idx].cmd    = cmd;
  ff[idx].online = true;
  ff[idx].lastMs = millis();
}

// ── Send ACK to Node ─────────────────────────────────────
void sendAck(int id) {
  String ack = "ACK:ID:" + String(id) + ",STS:RECEIVED";
  LoRa.beginPacket();
  LoRa.print(ack);
  LoRa.endPacket();
  LoRa.receive();
  Serial.println("[TX ACK] " + ack);
}

// ── Broadcast Alert to All Field Nodes ───────────────────
void broadcastAll(String msg) {
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
  LoRa.receive();
  Serial.println("[TX BCAST] " + msg);
}

// ── Buzzer Control ────────────────────────────────────────
void activateBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerStop = millis() + BUZZER_DURATION;
}

void checkBuzzerTimer() {
  if (buzzerStop > 0 && millis() >= buzzerStop) {
    buzzerStop = 0;
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ── Check Node Offline Status ────────────────────────────
void checkOfflineStatus() {
  for (int i = 0; i < 2; i++) {
    if (ff[i].online && millis() - ff[i].lastMs > OFFLINE_TIMEOUT) {
      ff[i].online = false;
      Serial.println("[OFFLINE] Node #" + String(ff[i].id));
    }
  }
}

// ── HTTP: Dashboard Page ──────────────────────────────────
void handleDashboard() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

// ── HTTP: JSON Data Endpoint ─────────────────────────────
void handleData() {
  String json = "{";
  json += "\"alertColor\":\"" + alertColor + "\",";
  json += "\"lastAlert\":\""  + lastAlert  + "\",";
  json += "\"ff\":[";
  for (int i = 0; i < 2; i++) {
    unsigned long seenSec = ff[i].lastMs > 0 ? (millis() - ff[i].lastMs) / 1000 : 0;
    json += "{";
    json += "\"id\":"     + String(ff[i].id)    + ",";
    json += "\"hr\":"     + String(ff[i].hr)    + ",";
    json += "\"cmd\":\""  + ff[i].cmd           + "\",";
    json += "\"status\":\""+ (ff[i].online ? "ONLINE" : "OFFLINE") + "\",";
    json += "\"lastSeen\":"+ String(seenSec);
    json += "}";
    if (i < 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
  // Reset alert colour once acknowledged
  alertColor = "green";
}

// ── HTTP: Silence Server Buzzer ───────────────────────────
void handleBuzzerSilence() {
  digitalWrite(BUZZER_PIN, LOW);
  buzzerStop = 0;
  server.send(200, "text/plain", "Buzzer silenced.");
  Serial.println(F("[BUZZER] Silenced via dashboard"));
}

// ── String Helpers ────────────────────────────────────────
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
