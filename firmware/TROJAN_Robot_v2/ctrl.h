#pragma once
// The robot controller: decides, every 20 ms, which source drives the legs
// (move/keyframes > pose lab > drive trick / stick through the gait > posture) and runs the personality.
#include "gait.h"
#include "anim.h"
#include "face.h"

enum PostureKind : uint8_t { P_SEATED, P_STAND, P_HOLD };

// The motion task (50 Hz, highest priority on core 1) and the web/serial handlers (loop task) both touch
// Ctrl. Everything that reads or changes Ctrl holds this lock; nothing slow (network sends, serial
// printing) happens while it is held, so the legs never wait for Wi-Fi.
static SemaphoreHandle_t g_ctlMtx = nullptr;
struct CtlLock {
  CtlLock() { if (g_ctlMtx) xSemaphoreTakeRecursive(g_ctlMtx, portMAX_DELAY); }
  ~CtlLock() { if (g_ctlMtx) xSemaphoreGiveRecursive(g_ctlMtx); }
};

enum Src : uint8_t { SRC_ANIM, SRC_MANUAL, SRC_POSTURE, SRC_GAIT };
static const float FAST_SLEW = 800;      // deg/s for moves and the run gait (SG90 tops out near 600)
static const uint32_t XFADE_MS = 300;    // every switch between motion sources is cross-faded

// Drive tricks: short scripts fed to the walking engine (same gait that walks straight).
// x forward, y left, w turn left, in percent of full stick.
struct DStep { uint16_t ms; int8_t x, y, w; };
struct DTrick { const char* name; const char* label; const DStep* st; uint8_t n; uint8_t face; };
static const DStep T_SPIN[] = {{3400, 0, 0, 100}};
static const DStep T_CIRCLE[] = {{6500, 70, 0, 60}};
static const DStep T_ZIG[] = {{1100, 60, 70, 0}, {1100, 60, -70, 0}, {1100, 60, 70, 0}, {1100, 60, -70, 0}};
static const DStep T_EIGHT[] = {{5200, 70, 0, 65}, {5200, 70, 0, -65}};
static const DTrick DTRICKS[] = {
  {"spin", "Spin", T_SPIN, 1, FC_DIZZY}, {"circle", "Circle", T_CIRCLE, 1, FC_HAPPY},
  {"zigzag", "Zigzag", T_ZIG, 4, FC_EXCITED}, {"figure8", "Figure 8", T_EIGHT, 2, FC_COOL}};
static const int DTRICK_N = sizeof(DTRICKS) / sizeof(DTRICKS[0]);
static const DTrick* findTrick(const String& n) {
  for (int i = 0; i < DTRICK_N; i++) if (n == DTRICKS[i].name) return &DTRICKS[i];
  return nullptr;
}

struct Ctrl;
static Ctrl* g_ctrl = nullptr;

struct Ctrl {
  Cfg cfg;
  ServoBus bus;
  Gait gait;
  Anim anim;
  Face face;

  Pose out = makePose(0, 75);
  Pose posture = makePose(0, 75);
  Pose manual = makePose();
  PostureKind pk = P_SEATED;

  float cx = 0, cy = 0, cw = 0;
  bool boost = false;            // hold-to-run from the remote
  float tempo = 1.0f;            // dance speed (not saved)
  const DTrick* trick = nullptr;
  uint32_t trickT0 = 0;
  bool runFace = false;
  bool idleMove = false;         // the running move was started by the idle personality
  Src src = SRC_POSTURE;
  Pose xfFrom = makePose(0, 75);
  uint32_t xfT0 = 0;
  // diagnostics (shown in the remote's System tab)
  uint32_t lateTicks = 0, worstGapMs = 0;
  uint8_t resetReason = 0;
  uint32_t lastCtl = 0, lastTick = 0, lastActivity = 0;
  uint32_t slowUntil = 0, standUntil = 0, manualUntil = 0, faceRevertAt = 0;
  uint32_t rawUntil[SERVO_N] = {0};
  float rawVal[SERVO_N] = {0};
  bool limp = false, sleeping = false;
  bool playlist = false;
  int playIdx = 0;
  uint32_t playlistNext = 0;
  uint32_t nextLook = 0, nextIdle = 0, connectedUntil = 0, lastPoll = 0, lastPoll2 = 0;
  int lastClients = 0;
  bool wantWifiScreen = false;

  static void faceCb(uint8_t f) { if (g_ctrl) g_ctrl->face.show(f); }
  static void titleCb(const char* t) { if (g_ctrl) g_ctrl->face.titleCard(t); }

  // ---------------------------------------------------------------- lifecycle
  void begin() {
    g_ctrl = this;
    cfgLoad(cfg);
    if (cfg.mode >= GM_N) cfg.mode = GM_CRAWL;
    bus.begin(posture);
    applyCfg();
    bus.setPose(posture);
    for (int i = 0; i < SERVO_N; i++) bus.cur[i] = bus.tgt[i];
    bus.write(true);
    anim.onFace = faceCb;
    anim.onTitle = titleCb;
    lastActivity = millis();
  }

  void applyCfg() {
    bus.maxSpeed = constrain(cfg.maxSpeed, 60.0f, 900.0f);
    for (int i = 0; i < SERVO_N; i++) bus.trim[i] = cfg.servoTrim[i];
  }

  // Any command counts as activity: wakes servos and the face.
  void touch() {
    uint32_t now = millis();
    lastActivity = now;
    if (!bus.attached) bus.attachAll();
    limp = false;
    if (face.id == FC_WIFI || face.id == FC_CONNECTED || face.id == FC_BOOT) face.back();
    if (sleeping) {
      sleeping = false;
      face.show(FC_SURPRISED);
      faceRevertAt = now + 1200;
    }
  }

  void startup() {
    std::vector<KF> v;
    addKF(v, 1100, 0, 0, 0, M_ALL, 30, 0);
    addKF(v, 800, 0, 0, 0, 0, 0, 0);
    addKF(v, 300, +16, 0, 0, 0, 0, 0);
    addKF(v, 300, -16, 0, 0, 0, 0, 0);
    addKF(v, 300, 0, 0, 0, 0, 0, 0);
    addKF(v, 350, 0, 0, 0, M_ALL, 22, 0);
    addKF(v, 350, 0, 0, 0, 0, 0, 0);
    pk = P_STAND;
    anim.startKF("startup", v, out);
    anim.cancellable = false;
    wantWifiScreen = true;
  }

  // ---------------------------------------------------------------- commands
  void stopAll() {
    if (trick) face.back();
    anim.stop();
    playlist = false;
    trick = nullptr;
    gait.reset();
  }

  void ctl(float x, float y, float w, bool b = false) {
    touch();
    cx = constrain(x, -1.0f, 1.0f);
    cy = constrain(y, -1.0f, 1.0f);
    cw = constrain(w, -1.0f, 1.0f);
    boost = b;
    lastCtl = millis();
  }

  void stand() {
    touch();
    uint32_t now = millis();
    stopAll();
    pk = P_STAND;
    slowUntil = now + 1200;
    standUntil = now + 1100;
  }
  void sit() {
    touch();
    stopAll();
    pk = P_SEATED;
    slowUntil = millis() + 1400;
    cx = cy = cw = 0;
  }
  void relax() {          // release the servos completely (legs go limp)
    stopAll();
    cx = cy = cw = 0;
    bus.detachAll();
    limp = true;
  }
  void estop() {
    uint32_t now = millis();
    touch();
    stopAll();
    cx = cy = cw = 0;
    manualUntil = 0;
    for (int i = 0; i < SERVO_N; i++) rawUntil[i] = 0;
    pk = P_STAND;
    standUntil = now + 900;
    slowUntil = 0;
    face.show(FC_SURPRISED);
    faceRevertAt = now + 1500;
  }

  // Prepare for a new motion: the new one replaces whatever was running.
  void prep(bool fromPlaylist) {
    touch();
    idleMove = false;
    if (!fromPlaylist) playlist = false;
    if (trick) { trick = nullptr; face.back(); }
    cx = cy = cw = 0;
    gait.reset();
    manualUntil = 0;
  }

  bool play(const String& name, bool fromPlaylist = false) {
    if (name.startsWith("u:")) {
      File f = LittleFS.open("/d_" + cleanName(name.substring(2)) + ".txt", "r");
      if (!f) return false;
      String t = f.readString();
      f.close();
      std::vector<KF> v;
      if (!parseDance(t, v)) return false;
      return startKF(name, v, fromPlaylist);
    }
    if (const Move* m = findMove(name)) {
      prep(fromPlaylist);
      pk = P_STAND;
      anim.startMove(m, out, tempo, cfg.height);
      anim.cancellable = true;
      return true;
    }
    if (const DTrick* d = findTrick(name)) { startTrick(d); return true; }
    return false;
  }

  bool startKF(const String& name, std::vector<KF>& v, bool fromPlaylist = false) {
    if (v.empty()) return false;
    prep(fromPlaylist);
    if (pk == P_SEATED) {                    // get up first
      std::vector<KF> pre;
      addKF(pre, 1000, 0, 0, 0, 0, 0, cfg.height);
      v.insert(v.begin(), pre[0]);
    }
    pk = P_STAND;
    anim.startKF(name, v, out);
    anim.cancellable = true;
    return true;
  }

  void startTrick(const DTrick* d) {
    prep(false);
    anim.stop();
    uint32_t now = millis();
    trickT0 = now;
    if (pk == P_SEATED) { pk = P_STAND; slowUntil = now + 1200; standUntil = now + 1100; trickT0 = standUntil; }
    else if (pk == P_HOLD) { pk = P_STAND; }
    trick = d;
    face.show(d->face);
  }

  void startPlaylist() {
    touch();
    playlist = true;
    playIdx = 0;
    playNext();
  }
  void playNext() {
    for (int tries = 0; tries < MOVE_N; tries++) {
      const Move& m = MOVES[playIdx % MOVE_N];
      playIdx++;
      if (m.kind == 1) { play(m.name, true); return; }
    }
    playlist = false;
  }

  void poseLab(const float s[LEG_N], const float a[LEG_N]) {
    touch();
    stopAll();
    for (int i = 0; i < LEG_N; i++) { manual.s[i] = s[i]; manual.a[i] = a[i]; }
    manualUntil = millis() + 15000;   // the pose holds 15 s after the last slider move
    cx = cy = cw = 0;
  }
  void rawTest(uint8_t i, float v) {
    if (i >= SERVO_N) return;
    touch();
    rawVal[i] = v;
    rawUntil[i] = millis() + 6000;
  }

  // ---------------------------------------------------------------- the 50 Hz brain
  void update(uint32_t now) {
    uint32_t dtms = lastTick ? now - lastTick : 20;
    lastTick = now;
    if (dtms > 30) lateTicks++;
    if (dtms > worstGapMs) worstGapMs = dtms;
    if (dtms > 100) dtms = 100;
    float dt = dtms / 1000.0f;

    bool fresh = (now - lastCtl) < cfg.deadMs;            // dead-man: stale stick = no stick
    float x = fresh ? cx : 0, y = fresh ? cy : 0, w = fresh ? cw : 0;
    bool stick = fabsf(x) > 0.08f || fabsf(y) > 0.08f || fabsf(w) > 0.08f;
    if (!stick) x = y = w = 0;

    uint8_t mode = cfg.mode;
    if (boost && fresh && mode < GM_RUN) mode = GM_RUN;
    if (trick && stick) { trick = nullptr; face.back(); }  // the sticks always win
    if (trick && (int32_t)(now - trickT0) >= 0) {
      uint32_t e = now - trickT0;
      const DStep* st = nullptr;
      for (uint8_t i = 0; i < trick->n; i++) {
        if (e < trick->st[i].ms) { st = &trick->st[i]; break; }
        e -= trick->st[i].ms;
      }
      if (st) { x = st->x / 100.0f; y = st->y / 100.0f; w = st->w / 100.0f; mode = cfg.mode == GM_RUN ? GM_RUN : GM_TROT; }
      else { trick = nullptr; face.back(); }
    }
    bool want = fabsf(x) > 0.08f || fabsf(y) > 0.08f || fabsf(w) > 0.08f;

    if (pk == P_STAND) posture = makePose(0, cfg.height);
    else if (pk == P_SEATED) posture = makePose(0, 75);

    Pose target = posture;
    if (anim.active && stick && anim.cancellable) { anim.stop(); playlist = false; face.back(); }
    bool fast = false, running = false;

    Src now_src;
    if (anim.active) {
      now_src = SRC_ANIM;
      Pose p = out;
      bool going = anim.tick(dtms, p);
      target = p;
      fast = true;
      if (!going) {                                    // finished: hold the last pose
        posture = p;
        float m = 0;
        for (int i = 0; i < LEG_N; i++) m = max(m, max(fabsf(p.s[i]), fabsf(p.a[i] - cfg.height)));
        pk = m < 1.0f ? P_STAND : P_HOLD;     // finished at neutral -> normal standing posture
        bool wasStartup = anim.name == "startup";
        if (!idleMove) lastActivity = now;   // a move you asked for counts as activity until it ends
        idleMove = false;
        face.back();
        if (wasStartup && wantWifiScreen) { face.show(FC_WIFI); wantWifiScreen = false; }
        if (playlist) playlistNext = now + 1500;
      }
    } else if (now < manualUntil) {
      now_src = SRC_MANUAL;
      target = manual;
    } else if (now < standUntil) {
      now_src = SRC_POSTURE;
      target = posture;
    } else if (want && pk == P_SEATED) {
      now_src = SRC_POSTURE;
      stand();
      target = posture;
    } else {
      if (pk == P_HOLD && want) { pk = P_STAND; posture = makePose(0, cfg.height); }
      gait.update(dt, x, y, w, cfg, mode);
      bool walking = want || gait.mag > 0.02f;
      now_src = walking ? SRC_GAIT : SRC_POSTURE;
      target = walking ? gait.out : posture;
      running = walking && mode >= GM_RUN;
      fast = running;
      if (want && face.isEyes()) { face.lookAt(-(y + w) * 0.9f, -0.25f * x); nextLook = now + 1500; }   // eyes lead the way
    }

    // focused face while running
    if (running && !runFace && face.isEyes() && face.id == face.mood) { face.show(FC_DETERMINED); runFace = true; }
    else if (!running && runFace) { runFace = false; if (face.id == FC_DETERMINED) face.back(); }

    // Cross-fade on every source change (move cancelled by the stick, E-STOP mid-dance, Pose Lab
    // released...), so a switch never makes the legs jump. Moves already fade in by themselves.
    if (now_src != src) { xfFrom = out; xfT0 = now; src = now_src; }
    if (src != SRC_ANIM && now - xfT0 < XFADE_MS) {
      float u = ease((now - xfT0) / (float)XFADE_MS);
      for (int i = 0; i < LEG_N; i++) {
        target.s[i] = xfFrom.s[i] + (target.s[i] - xfFrom.s[i]) * u;
        target.a[i] = xfFrom.a[i] + (target.a[i] - xfFrom.a[i]) * u;
      }
    }

    out = target;
    bus.setPose(out);
    for (int i = 0; i < SERVO_N; i++) if (now < rawUntil[i]) bus.setRaw(i, rawVal[i]);
    // moves and the run gait get the servos' full speed; walking keeps the tuned limit
    bus.maxSpeed = (now < slowUntil) ? 140.0f : fast ? max(cfg.maxSpeed, FAST_SLEW) : cfg.maxSpeed;
    bus.tick(dt);

    behaviours(now);
  }

  // ---------------------------------------------------------------- personality
  void behaviours(uint32_t now) {
    if (faceRevertAt && now > faceRevertAt) { faceRevertAt = 0; face.back(); }

    // WiFi welcome flow
    if (face.id == FC_WIFI && now - lastPoll > 300) {
      lastPoll = now;
      if (WiFi.softAPgetStationNum() > 0) { face.show(FC_CONNECTED); connectedUntil = now + 2600; }
    } else if (face.id == FC_CONNECTED && now > connectedUntil) {
      face.back();
      if (cfg.react && !anim.active && pk == P_STAND) play("wave");
    }
    if (now - lastPoll2 > 1000) {         // later connections get a quick greeting
      lastPoll2 = now;
      int n = WiFi.softAPgetStationNum();
      if (n > lastClients && cfg.react && face.isEyes() && !anim.active && !sleeping) {
        face.banner("Hello!", 1800);
        face.show(FC_HAPPY);
        faceRevertAt = now + 2500;
      }
      lastClients = n;
    }

    if (playlist && !anim.active && playlistNext && now > playlistNext) { playlistNext = 0; playNext(); }

    if (anim.active || playlist || trick || stickActive(now)) return;

    // sleep
    if (cfg.sleepSec && !sleeping && now - lastActivity > (uint32_t)cfg.sleepSec * 1000UL) {
      sleeping = true;
      pk = P_SEATED;
      slowUntil = now + 1400;
      face.setMood(FC_SLEEPY);
    }
    if (sleeping) { relaxIfSettled(now); return; }

    // gaze and mood glances
    if (face.isEyes() && now > nextLook) {
      nextLook = now + random(1200, 3600);
      face.lookAt(random(-100, 101) / 100.0f, random(-60, 61) / 100.0f);
      if (random(0, 10) == 0) face.lookAt(0, 0);
    }
    if (cfg.life && now > nextIdle) {
      nextIdle = now + random(22000, 50000);
      if (face.isEyes() && face.id == face.mood && pk == P_STAND && now - lastActivity > 15000) {
        uint32_t la = lastActivity;   // idle gestures must not count as activity (or TROJAN never sleeps)
        idleMove = false;
        switch (random(0, 10)) {
          case 0: play("look"); break;
          case 1: play("stretch"); break;
          case 2: play("wiggle"); break;
          case 3: play("tap"); break;
          case 4: play("scratch"); break;
          case 5: play("sneeze"); break;
          case 6: face.show(FC_CURIOUS); faceRevertAt = now + 2200; break;
          case 7: face.show(FC_BORED); faceRevertAt = now + 2600; break;
          case 8: face.show(FC_WINK); faceRevertAt = now + 1400; break;
          default: face.show(FC_HAPPY); faceRevertAt = now + 1800; break;
        }
        lastActivity = la;
        idleMove = anim.active;
      }
    }
    relaxIfSettled(now);
  }

  bool stickActive(uint32_t now) const {
    return (now - lastCtl) < cfg.deadMs && (fabsf(cx) > 0.08f || fabsf(cy) > 0.08f || fabsf(cw) > 0.08f);
  }

  void relaxIfSettled(uint32_t now) {
    if (pk == P_SEATED && bus.attached && bus.settled() && now - lastActivity > (uint32_t)cfg.relaxSec * 1000UL) bus.detachAll();
  }

  // ---------------------------------------------------------------- state for the remote
  String stateJson(uint32_t now) {
    static const char* PK[] = {"seated", "stand", "hold"};
    String j = "{\"up\":" + String(now / 1000);
    j += ",\"pk\":\"" + String(PK[pk]) + "\"";
    j += ",\"anim\":\"" + (anim.active ? anim.name : String("")) + "\"";
    j += ",\"trick\":\"" + String(trick ? trick->name : "") + "\"";
    j += ",\"mode\":" + String(cfg.mode) + ",\"run\":" + String(runFace ? 1 : 0);
    j += ",\"playlist\":" + String(playlist ? 1 : 0);
    j += ",\"limp\":" + String(bus.attached ? 0 : 1);
    j += ",\"sleep\":" + String(sleeping ? 1 : 0);
    j += ",\"face\":" + String(face.id) + ",\"mood\":" + String(face.mood);
    j += ",\"clients\":" + String(WiFi.softAPgetStationNum());
    j += ",\"heap\":" + String(ESP.getFreeHeap());
    j += ",\"rst\":" + String(resetReason) + ",\"late\":" + String(lateTicks) + ",\"gap\":" + String(worstGapMs);
    j += ",\"v\":[" + String(gait.vx, 2) + "," + String(gait.vy, 2) + "," + String(gait.wz, 2) + "]";
    j += ",\"s\":[";
    for (int i = 0; i < LEG_N; i++) j += String(out.s[i], 1) + (i < 3 ? "," : "");
    j += "],\"a\":[";
    for (int i = 0; i < LEG_N; i++) j += String(out.a[i], 1) + (i < 3 ? "," : "");
    j += "],\"raw\":[";
    for (int i = 0; i < SERVO_N; i++) j += String(bus.cur[i], 0) + (i < SERVO_N - 1 ? "," : "");
    j += "]}";
    return j;
  }
};
