#pragma once
// HTTP API behind the remote. Everything is a small GET/POST so any browser can drive TROJAN.
#include <WebServer.h>
#include "remote.h"
#include "ctrl.h"

extern Ctrl C;
extern WebServer server;

static float argF(const char* k, float d = 0) { return server.hasArg(k) ? server.arg(k).toFloat() : d; }
static void sendOk() { server.send(200, "text/plain", "ok"); }
static void sendJson(const String& s) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", s);
}

static String arrJson(const float* a, int n, int dec) {
  String j = "[";
  for (int i = 0; i < n; i++) j += String(a[i], dec) + (i < n - 1 ? "," : "");
  return j + "]";
}

static String cfgJson() {
  Cfg& c = C.cfg;
  String j = "{\"mode\":" + String(c.mode) + ",\"freq\":" + String(c.freq, 2) + ",\"stride\":" + String(c.stride, 1) +
             ",\"lift\":" + String(c.lift, 1) + ",\"height\":" + String(c.height, 1) + ",\"steer\":" + String(c.steer, 2) +
             ",\"maxSpeed\":" + String(c.maxSpeed, 0) + ",\"deadMs\":" + String(c.deadMs) + ",\"life\":" + String(c.life) +
             ",\"react\":" + String(c.react) + ",\"sleepSec\":" + String(c.sleepSec) + ",\"relaxSec\":" + String(c.relaxSec);
  j += ",\"ts\":" + arrJson(c.trimStride, 4, 2) + ",\"th\":" + arrJson(c.trimHip, 4, 1) + ",\"tl\":" + arrJson(c.trimLift, 4, 2) + ",\"st\":[";
  for (int i = 0; i < SERVO_N; i++) j += String(c.servoTrim[i]) + (i < SERVO_N - 1 ? "," : "");
  return j + "]}";
}

static void hRoot() {
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html", REMOTE_HTML);
}

static void hCtl() { C.ctl(argF("x"), argF("y"), argF("w")); sendOk(); }
static void hState() { sendJson(C.stateJson(millis())); }

static void hAct() {
  String a = server.arg("a");
  if (a == "stand") C.stand();
  else if (a == "sit") C.sit();
  else if (a == "relax") C.relax();
  else if (a == "estop") C.estop();
  else { server.send(400, "text/plain", "bad action"); return; }
  sendOk();
}

static void hPlay() {
  if (server.hasArg("show")) { C.startPlaylist(); sendOk(); return; }
  if (C.play(server.arg("n"))) sendOk(); else server.send(404, "text/plain", "no such move");
}

static void hCfg() { sendJson(cfgJson()); }

static void hSet() {
  Cfg& c = C.cfg;
  if (server.hasArg("mode")) c.mode = server.arg("mode").toInt() ? 1 : 0;
  if (server.hasArg("freq")) c.freq = constrain(argF("freq"), 0.3f, 2.6f);
  if (server.hasArg("stride")) c.stride = constrain(argF("stride"), 4.0f, 38.0f);
  if (server.hasArg("lift")) c.lift = constrain(argF("lift"), 6.0f, 50.0f);
  if (server.hasArg("height")) c.height = constrain(argF("height"), 0.0f, 40.0f);
  if (server.hasArg("steer")) c.steer = constrain(argF("steer"), -0.5f, 0.5f);
  if (server.hasArg("maxSpeed")) c.maxSpeed = constrain(argF("maxSpeed"), 60.0f, 900.0f);
  if (server.hasArg("deadMs")) c.deadMs = constrain(server.arg("deadMs").toInt(), 200, 2000);
  if (server.hasArg("life")) c.life = server.arg("life").toInt() ? 1 : 0;
  if (server.hasArg("react")) c.react = server.arg("react").toInt() ? 1 : 0;
  if (server.hasArg("sleepSec")) c.sleepSec = constrain(server.arg("sleepSec").toInt(), 0, 3600);
  if (server.hasArg("relaxSec")) c.relaxSec = constrain(server.arg("relaxSec").toInt(), 2, 300);
  for (int i = 0; i < 4; i++) {
    String n = String(i);
    if (server.hasArg("ts" + n)) c.trimStride[i] = constrain(argF(("ts" + n).c_str()), 0.4f, 1.6f);
    if (server.hasArg("th" + n)) c.trimHip[i] = constrain(argF(("th" + n).c_str()), -15.0f, 15.0f);
    if (server.hasArg("tl" + n)) c.trimLift[i] = constrain(argF(("tl" + n).c_str()), 0.4f, 1.8f);
  }
  for (int i = 0; i < SERVO_N; i++)
    if (server.hasArg("st" + String(i))) c.servoTrim[i] = constrain(server.arg("st" + String(i)).toInt(), -15, 15);
  C.applyCfg();
  C.touch();
  sendOk();
}
static void hSave() { cfgSave(C.cfg); sendOk(); }
static void hReset() { C.cfg = Cfg(); C.applyCfg(); cfgSave(C.cfg); sendOk(); }

static void hPose() {
  float s[4], a[4];
  for (int i = 0; i < 4; i++) { s[i] = argF(("s" + String(i)).c_str()); a[i] = argF(("a" + String(i)).c_str()); }
  C.poseLab(s, a);
  sendOk();
}
static void hRaw() { C.rawTest(server.arg("i").toInt(), argF("v", 90)); sendOk(); }

static void hMood() { C.touch(); C.face.setMood(server.arg("m").toInt()); sendOk(); }
static void hScreen() { C.touch(); C.face.show(server.arg("id").toInt()); sendOk(); }
static void hText() { C.touch(); C.face.banner(server.arg("t").substring(0, 60), constrain(server.arg("ms").toInt(), 500, 20000)); sendOk(); }

// ---- Face Studio (1024-byte bitmaps as 2048 hex chars)
static bool hexDecode(const String& h, uint8_t* out, size_t n) {
  if (h.length() < n * 2) return false;
  for (size_t i = 0; i < n; i++) {
    char a = h[i * 2], b = h[i * 2 + 1];
    auto v = [](char c) -> int { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1; };
    int x = v(a), y = v(b);
    if (x < 0 || y < 0) return false;
    out[i] = (x << 4) | y;
  }
  return true;
}
static void hFShow() {
  static uint8_t buf[1024];
  if (!hexDecode(server.arg("plain"), buf, 1024)) { server.send(400, "text/plain", "bad bitmap"); return; }
  C.touch();
  C.face.showRaw(buf);
  sendOk();
}
static void hFSave() {
  static uint8_t buf[1024];
  int slot = server.arg("slot").toInt();
  if (slot < 0 || slot > 7 || !hexDecode(server.arg("plain"), buf, 1024)) { server.send(400, "text/plain", "bad request"); return; }
  if (!C.face.saveSlot(slot, buf)) { server.send(500, "text/plain", "write failed"); return; }
  sendOk();
}
static void hFGet() {
  int slot = server.arg("slot").toInt();
  File f = LittleFS.open(Face::slotPath(slot), "r");
  if (!f || f.size() != 1024) { server.send(404, "text/plain", "empty"); return; }
  String h;
  h.reserve(2048);
  for (int i = 0; i < 1024; i++) { char b[3]; snprintf(b, 3, "%02x", f.read()); h += b; }
  f.close();
  server.send(200, "text/plain", h);
}
static void hFDel() { LittleFS.remove(Face::slotPath(server.arg("slot").toInt())); sendOk(); }
static void hFList() {
  String j = "[";
  bool first = true;
  for (int i = 0; i < 8; i++) if (C.face.slotExists(i)) { j += (first ? "" : ",") + String(i); first = false; }
  sendJson(j + "]");
}

// ---- dances
static String userDanceList() {
  String j = "[";
  bool first = true;
  File root = LittleFS.open("/");
  for (File f = root.openNextFile(); f; f = root.openNextFile()) {
    String n = f.name();
    if (n.startsWith("/")) n = n.substring(1);
    if (n.startsWith("d_") && n.endsWith(".txt")) {
      j += String(first ? "" : ",") + "\"" + n.substring(2, n.length() - 4) + "\"";
      first = false;
    }
  }
  return j + "]";
}
static void hDances() {
  String j = "{\"builtin\":[";
  bool first = true;
  for (int i = 0; i < ANIM_N; i++) {
    if (!ANIMS[i].dance) continue;
    std::vector<KF> v;
    ANIMS[i].build(v);
    uint32_t t = 0;
    for (auto& k : v) t += k.ms;
    j += String(first ? "" : ",") + "{\"n\":\"" + ANIMS[i].name + "\",\"t\":" + String(t / 1000) + "}";
    first = false;
  }
  sendJson(j + "],\"user\":" + userDanceList() + "}");
}
static void hDGet() {
  File f = LittleFS.open("/d_" + cleanName(server.arg("n")) + ".txt", "r");
  if (!f) { server.send(404, "text/plain", "none"); return; }
  server.send(200, "text/plain", f.readString());
  f.close();
}
static void hDSave() {
  String n = cleanName(server.arg("n"));
  String body = server.arg("plain");
  std::vector<KF> v;
  if (!n.length() || body.length() > 16000 || !parseDance(body, v)) { server.send(400, "text/plain", "bad dance"); return; }
  File f = LittleFS.open("/d_" + n + ".txt", "w");
  if (!f) { server.send(500, "text/plain", "write failed"); return; }
  f.print(body);
  f.close();
  sendOk();
}
static void hDDel() { LittleFS.remove("/d_" + cleanName(server.arg("n")) + ".txt"); sendOk(); }
static void hDPlay() {
  std::vector<KF> v;
  if (!parseDance(server.arg("plain"), v)) { server.send(400, "text/plain", "bad dance"); return; }
  C.startAnim("preview", v, true);
  sendOk();
}

static void hSys() {
  String a = server.arg("a");
  if (a == "reboot") { sendOk(); delay(200); ESP.restart(); }
  else server.send(400, "text/plain", "bad");
}

// Captive portal: any unknown URL (phone connectivity probes included) lands on the remote.
static void hNotFound() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
  server.send(302, "text/plain", "");
}

static void webBegin() {
  server.on("/", hRoot);
  server.on("/api/c", hCtl);
  server.on("/api/s", hState);
  server.on("/api/a", hAct);
  server.on("/api/play", hPlay);
  server.on("/api/cfg", hCfg);
  server.on("/api/set", hSet);
  server.on("/api/save", hSave);
  server.on("/api/reset", hReset);
  server.on("/api/pose", hPose);
  server.on("/api/raw", hRaw);
  server.on("/api/mood", hMood);
  server.on("/api/screen", hScreen);
  server.on("/api/text", hText);
  server.on("/api/fshow", HTTP_POST, hFShow);
  server.on("/api/fsave", HTTP_POST, hFSave);
  server.on("/api/fget", hFGet);
  server.on("/api/fdel", hFDel);
  server.on("/api/flist", hFList);
  server.on("/api/dances", hDances);
  server.on("/api/dget", hDGet);
  server.on("/api/dsave", HTTP_POST, hDSave);
  server.on("/api/ddel", hDDel);
  server.on("/api/dplay", HTTP_POST, hDPlay);
  server.on("/api/sys", hSys);
  server.onNotFound(hNotFound);
  server.begin();
}
