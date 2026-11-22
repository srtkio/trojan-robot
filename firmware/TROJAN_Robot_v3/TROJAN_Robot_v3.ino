// TROJAN - quadruped firmware. Written from scratch for this robot.
// Wi-Fi: Sesame-Controller / 12345678, remote at http://192.168.4.1 (or http://trojan.local).
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <LittleFS.h>
#include "config.h"
#include "ctrl.h"
#include "fastserver.h"

Ctrl C;
FastServer server(80);
DNSServer dns;

#include "web.h"

static const uint32_t TICK_MS = 20;   // 50 Hz control loop

static uint32_t nextTick = 0;
// shortcut: servo updates still run in loop(); moving them to their own task needs every web handler
// wrapped in CtlLock first (see ctrl.h). Until then the lock is a no-op (mutex never created).

// Tiny serial console for bench testing without Wi-Fi:
//   c x y w | stand | sit | relax | estop | play name | raw i v | mode 0-2 | list | s
static void serialCli() {
  static char buf[64];
  static uint8_t n = 0;
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch != '\n' && ch != '\r') { if (n < sizeof(buf) - 1) buf[n++] = ch; continue; }
    buf[n] = 0;
    n = 0;
    String l = String(buf);
    l.trim();
    if (!l.length()) continue;
    String reply;
    {
      CtlLock lk;
      float a = 0, b = 0, c = 0;
      int i = 0;
      char name[32];
      if (sscanf(l.c_str(), "c %f %f %f", &a, &b, &c) == 3) { C.ctl(a, b, c); reply = "ctl ok"; }
      else if (l == "stand") { C.stand(); reply = ("stand"); }
      else if (l == "sit") { C.sit(); reply = ("sit"); }
      else if (l == "estop") { C.estop(); reply = ("estop"); }
      else if (l == "relax") { C.relax(); reply = ("relax"); }
      else if (sscanf(l.c_str(), "play %31s", name) == 1) reply = (C.play(name) ? "playing" : "no such move");
      else if (sscanf(l.c_str(), "raw %d %f", &i, &a) == 2) { C.rawTest(i, a); reply = ("raw ok"); }
      else if (l == "s") reply = (C.stateJson(millis()));
      else if (l == "list") {
        for (int k = 0; k < MOVE_N; k++) { reply += MOVES[k].name; reply += MOVES[k].kind ? " (dance) " : " "; }
        for (int k = 0; k < DTRICK_N; k++) { reply += DTRICKS[k].name; reply += " (drive) "; }
      }
      else if (sscanf(l.c_str(), "mode %d", &i) == 1) { C.cfg.mode = constrain(i, 0, GM_N - 1); reply = ("mode ok"); }
      else reply = "?";
    }
    Serial.println(reply);
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());
  LittleFS.begin(true);
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

  C.begin();                                   // servos hold the seated pose first
  C.resetReason = (uint8_t)esp_reset_reason();  // 9 = brownout (power dip), 4 = crash, 1 = power on
  if (C.face.begin()) C.face.show(FC_BOOT);

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.setHostname(HOSTNAME);
  C.face.apPass = apPassLoad();
  WiFi.softAP(AP_SSID, C.face.apPass.c_str(), 1, 0, AP_MAX_CLIENTS);
  esp_wifi_set_inactive_time(WIFI_IF_AP, 30);   // a phone that walks away frees the single slot after 30 s (default 300)
  dns.start(53, "*", WiFi.softAPIP());         // captive portal: every name resolves to TROJAN
  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);
  webBegin();

  C.startup();                                 // stand up, look around, then show the Wi-Fi screen
  Serial.printf("%s %s ready. AP %s  http://%s\n", ROBOT_NAME, FW_VERSION, AP_SSID, WiFi.softAPIP().toString().c_str());
}

void loop() {
  dns.processNextRequest();
  server.handle();
  serialCli();

  uint32_t now = millis();
  if ((int32_t)(now - nextTick) >= 0) {
    nextTick = now + TICK_MS;
    C.update(now);
  }
}
