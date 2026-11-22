// TROJAN - quadruped firmware. Written from scratch for this robot.
// Wi-Fi: Sesame-Controller / 12345678, remote at http://192.168.4.1 (or http://trojan.local).
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <LittleFS.h>
#include "config.h"
#include "ctrl.h"

Ctrl C;
WebServer server(80);
DNSServer dns;

#include "web.h"

static const uint32_t TICK_MS = 20;   // 50 Hz control loop
static uint32_t nextTick = 0;

// Tiny serial console for bench testing without Wi-Fi:  c x y w | stand | sit | estop | play name | raw i v | s
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
    float a = 0, b = 0, c = 0;
    int i = 0;
    char name[32];
    if (sscanf(l.c_str(), "c %f %f %f", &a, &b, &c) == 3) { C.ctl(a, b, c); Serial.println("ctl ok"); }
    else if (l == "stand") { C.stand(); Serial.println("stand"); }
    else if (l == "sit") { C.sit(); Serial.println("sit"); }
    else if (l == "estop") { C.estop(); Serial.println("estop"); }
    else if (l == "relax") { C.relax(); Serial.println("relax"); }
    else if (sscanf(l.c_str(), "play %31s", name) == 1) Serial.println(C.play(name) ? "playing" : "no such move");
    else if (sscanf(l.c_str(), "raw %d %f", &i, &a) == 2) { C.rawTest(i, a); Serial.println("raw ok"); }
    else if (l == "s") Serial.println(C.stateJson(millis()));
    else Serial.println("?");
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());
  LittleFS.begin(true);
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

  C.begin();                                   // servos hold the seated pose first
  if (C.face.begin()) C.face.show(FC_BOOT);

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.setHostname(HOSTNAME);
  WiFi.softAP(AP_SSID, AP_PASS);
  dns.start(53, "*", WiFi.softAPIP());         // captive portal: every name resolves to TROJAN
  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);
  webBegin();

  C.startup();                                 // stand up, look around, then show the Wi-Fi screen
  Serial.printf("%s %s ready. AP %s  http://%s\n", ROBOT_NAME, FW_VERSION, AP_SSID, WiFi.softAPIP().toString().c_str());
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
  serialCli();

  uint32_t now = millis();
  if ((int32_t)(now - nextTick) >= 0) {
    nextTick = now + TICK_MS;
    C.update(now);
    C.face.tick(now);
  }
}
