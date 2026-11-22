#pragma once
// The robot controller: decides, every 20 ms, which source drives the legs
// (animation > pose lab > gait > posture) and runs the idle "personality".
#include "gait.h"
#include "anim.h"
#include "face.h"

enum PostureKind : uint8_t { P_SEATED, P_STAND, P_HOLD };

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

  // ---------------------------------------------------------------- lifecycle
  void begin() {
    g_ctrl = this;
    cfgLoad(cfg);
    bus.begin(posture);
    applyCfg();
    bus.setPose(posture);
    for (int i = 0; i < SERVO_N; i++) bus.cur[i] = bus.tgt[i];
    bus.write(true);
    anim.onFace = faceCb;
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
    anim.start("startup", v, out, false);
    anim.cancellable = false;
    wantWifiScreen = true;
  }

  // ---------------------------------------------------------------- commands
  void ctl(float x, float y, float w) {
    touch();
    cx = constrain(x, -1.0f, 1.0f);
    cy = constrain(y, -1.0f, 1.0f);
    cw = constrain(w, -1.0f, 1.0f);
    lastCtl = millis();
  }

  void stand() {
    touch();
    uint32_t now = millis();
    anim.stop();
    playlist = false;
    pk = P_STAND;
    slowUntil = now + 1200;
    standUntil = now + 1100;
    gait.reset();
  }
  void sit() {
    touch();
    anim.stop();
    playlist = false;
    pk = P_SEATED;
    slowUntil = millis() + 1400;
    gait.reset();
    cx = cy = cw = 0;
  }
  void relax() {          // release the servos completely (legs go limp)
    anim.stop();
    playlist = false;
    gait.reset();
    cx = cy = cw = 0;
    bus.detachAll();
    limp = true;
  }
  void estop() {
    uint32_t now = millis();
    touch();
    anim.stop();
    playlist = false;
    cx = cy = cw = 0;
    gait.reset();
    manualUntil = 0;
    for (int i = 0; i < SERVO_N; i++) rawUntil[i] = 0;
    pk = P_STAND;
    standUntil = now + 900;
    slowUntil = 0;
    face.show(FC_SURPRISED);
    faceRevertAt = now + 1500;
  }

  bool play(const String& name, bool fromPlaylist = false) {
    std::vector<KF> v;
    bool dance = true;
    if (name.startsWith("u:")) {
      File f = LittleFS.open("/d_" + cleanName(name.substring(2)) + ".txt", "r");
      if (!f) return false;
      String t = f.readString();
      f.close();
      if (!parseDance(t, v)) return false;
    } else {
      const NamedAnim* na = findAnim(name);
      if (!na) return false;
      na->build(v);
      dance = na->dance;
    }
    return startAnim(name, v, dance, fromPlaylist);
  }

  bool startAnim(const String& name, std::vector<KF>& v, bool dance, bool fromPlaylist = false) {
    touch();
    if (!fromPlaylist) playlist = false;
    if (v.empty()) return false;
    if (pk == P_SEATED) {                    // get up first
      std::vector<KF> pre;
      addKF(pre, 1000, 0, 0, 0, 0, 0, cfg.height);
      v.insert(v.begin(), pre[0]);
    }
    pk = P_STAND;
    cx = cy = cw = 0;
    gait.reset();
    anim.start(name, v, out, false);
    anim.cancellable = true;
    (void)dance;
    return true;
  }

  void startPlaylist() {
    touch();
    playlist = true;
    playIdx = 0;
    playNext();
  }
  void playNext() {
    int tries = 0;
    while (tries++ < ANIM_N) {
      const NamedAnim& n = ANIMS[playIdx % ANIM_N];
      playIdx++;
      if (n.dance) { play(n.name, true); return; }
    }
    playlist = false;
  }

  void poseLab(const float s[LEG_N], const float a[LEG_N]) {
    touch();
    if (anim.active) anim.stop();
    playlist = false;
    for (int i = 0; i < LEG_N; i++) { manual.s[i] = s[i]; manual.a[i] = a[i]; }
    manualUntil = millis() + 3500;
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
    if (dtms > 100) dtms = 100;
    float dt = dtms / 1000.0f;

    bool fresh = (now - lastCtl) < cfg.deadMs;            // dead-man: stale stick = no stick
    float x = fresh ? cx : 0, y = fresh ? cy : 0, w = fresh ? cw : 0;
    bool want = fabsf(x) > 0.08f || fabsf(y) > 0.08f || fabsf(w) > 0.08f;
    if (!want) x = y = w = 0;

    if (pk == P_STAND) posture = makePose(0, cfg.height);
    else if (pk == P_SEATED) posture = makePose(0, 75);

    Pose target = posture;
    if (anim.active && want && anim.cancellable) { anim.stop(); playlist = false; face.back(); }

    if (anim.active) {
      Pose p = out;
      bool running = anim.tick(dtms, p);
      target = p;
      if (!running) {                                    // finished: hold the last pose
        posture = p;
        float m = 0;
        for (int i = 0; i < LEG_N; i++) m = max(m, max(fabsf(p.s[i]), fabsf(p.a[i] - cfg.height)));
        pk = m < 1.0f ? P_STAND : P_HOLD;     // finished at neutral -> normal standing posture
        bool wasStartup = anim.name == "startup";
        face.back();
        if (wasStartup && wantWifiScreen) { face.show(FC_WIFI); wantWifiScreen = false; }
        if (playlist) playlistNext = now + 1500;
      }
    } else if (now < manualUntil) {
      target = manual;
    } else if (now < standUntil) {
      target = posture;
    } else if (want && pk == P_SEATED) {
      stand();
      target = posture;
    } else {
      if (pk == P_HOLD && want) { pk = P_STAND; posture = makePose(0, cfg.height); }
      gait.update(dt, x, y, w, cfg);
      target = (want || gait.mag > 0.02f) ? gait.out : posture;
    }

    out = target;
    bus.setPose(out);
    for (int i = 0; i < SERVO_N; i++) if (now < rawUntil[i]) bus.setRaw(i, rawVal[i]);
    bus.maxSpeed = (now < slowUntil) ? 140.0f : cfg.maxSpeed;
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

    if (anim.active || playlist || want_active(now)) return;

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
        switch (random(0, 5)) {
          case 0: play("look"); break;
          case 1: play("stretch"); break;
          case 2: play("wiggle"); break;
          case 3: face.show(FC_WINK); faceRevertAt = now + 1400; break;
          default: face.show(FC_HAPPY); faceRevertAt = now + 1800; break;
        }
        lastActivity = la;
      }
    }
    relaxIfSettled(now);
  }

  bool want_active(uint32_t now) const {
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
    j += ",\"playlist\":" + String(playlist ? 1 : 0);
    j += ",\"limp\":" + String(bus.attached ? 0 : 1);
    j += ",\"sleep\":" + String(sleeping ? 1 : 0);
    j += ",\"face\":" + String(face.id) + ",\"mood\":" + String(face.mood);
    j += ",\"clients\":" + String(WiFi.softAPgetStationNum());
    j += ",\"heap\":" + String(ESP.getFreeHeap());
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
