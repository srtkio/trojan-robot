#pragma once
// Keyframe player + built-in gestures and dances + user dance files.
#include <vector>
#include <LittleFS.h>
#include "robot.h"

// One keyframe = "reach this pose in ms". The first keyframe starts from wherever the robot is.
struct KF {
  uint16_t ms;
  float s[LEG_N];
  float a[LEG_N];
  uint8_t face;  // 0 = leave the face alone, else a face id (see face.h)
};

struct Anim {
  std::vector<KF> kf;
  bool active = false;
  bool loop = false;
  bool cancellable = true;  // a joystick push stops it
  String name;
  size_t idx = 0;
  uint32_t elapsed = 0;     // ms into the current segment
  Pose from;
  void (*onFace)(uint8_t) = nullptr;

  void start(const String& n, const std::vector<KF>& frames, const Pose& now, bool lp) {
    kf = frames;
    name = n;
    loop = lp && total() > 0;  // an all-zero-duration loop would spin forever
    idx = 0;
    elapsed = 0;
    from = now;
    active = !kf.empty();
    if (active) fireFace();
  }
  void stop() { active = false; name = ""; }

  void fireFace() { if (onFace && kf[idx].face) onFace(kf[idx].face); }

  uint32_t total() const { uint32_t t = 0; for (auto& k : kf) t += k.ms; return t; }

  // Advance by dtMs and write the interpolated pose. Returns false once the animation has finished
  // (out then holds the last keyframe).
  bool tick(uint32_t dtMs, Pose& out) {
    if (!active) return false;
    elapsed += dtMs;
    while (elapsed >= kf[idx].ms) {
      elapsed -= kf[idx].ms;
      for (int i = 0; i < LEG_N; i++) { from.s[i] = kf[idx].s[i]; from.a[i] = kf[idx].a[i]; }
      idx++;
      if (idx >= kf.size()) {
        if (loop) { idx = 0; } else { out = from; active = false; return false; }
      }
      fireFace();
    }
    float u = kf[idx].ms ? (float)elapsed / kf[idx].ms : 1.0f;
    u = u * u * (3 - 2 * u);
    for (int i = 0; i < LEG_N; i++) {
      out.s[i] = from.s[i] + (kf[idx].s[i] - from.s[i]) * u;
      out.a[i] = from.a[i] + (kf[idx].a[i] - from.a[i]) * u;
    }
    return true;
  }
};

// ---------------------------------------------------------------- builders
// Leg order everywhere: FR RR FL RL. Mask bit 0..3 = those legs.
#define M_FR 1
#define M_RR 2
#define M_FL 4
#define M_RL 8
#define M_ALL 15

// tw: twist the whole body (counter-clockwise +), fb: all legs forward +, dg: diagonal rock
// (FR+RL forward, RR+FL back), mask/lift: legs raised by `lift` deg, base: splay of all feet.
static void addKF(std::vector<KF>& v, uint16_t ms, float tw, float fb, float dg, uint8_t mask, float lift, float base, uint8_t face = 0) {
  static const float TW[LEG_N] = {+1, +1, -1, -1};
  static const float DG[LEG_N] = {+1, -1, -1, +1};
  KF k;
  k.ms = ms;
  k.face = face;
  for (int i = 0; i < LEG_N; i++) {
    k.s[i] = constrain(tw * TW[i] + fb + dg * DG[i], -MAX_SWING, MAX_SWING);
    k.a[i] = constrain(base + (((mask >> i) & 1) ? lift : 0), 0.0f, MAX_SPLAY);
  }
  v.push_back(k);
}

// Per-leg version for gestures that treat legs differently.
static void addLegs(std::vector<KF>& v, uint16_t ms, const float s[LEG_N], const float a[LEG_N], uint8_t face = 0) {
  KF k;
  k.ms = ms;
  k.face = face;
  for (int i = 0; i < LEG_N; i++) {
    k.s[i] = constrain(s[i], -MAX_SWING, MAX_SWING);
    k.a[i] = constrain(a[i], 0.0f, MAX_SPLAY);
  }
  v.push_back(k);
}

// Face ids used by the animations (kept in sync with face.h).
enum : uint8_t {
  F_NEUTRAL = 1, F_HAPPY, F_SAD, F_ANGRY, F_SURPRISED, F_SLEEPY, F_LOVE, F_EXCITED, F_WINK, F_DIZZY, F_SUSPICIOUS, F_COOL,
  F_EQ = 32, F_HEARTS, F_STARS, F_SPIRAL, F_GLITCH, F_NOTES, F_MATRIX
};

// ---- gestures (a few seconds each)
static void gWave(std::vector<KF>& v) {
  float s0[4] = {0, 0, 0, 0}, a0[4] = {75, 0, 0, 0};
  addLegs(v, 600, s0, a0, F_HAPPY);
  for (int i = 0; i < 4; i++) {
    float sa[4] = {+28, 0, 0, 0}, sb[4] = {-8, 0, 0, 0};
    addLegs(v, 260, sa, a0);
    addLegs(v, 260, sb, a0);
  }
  float z[4] = {0, 0, 0, 0};
  addLegs(v, 600, z, z, F_NEUTRAL);
}
static void gBow(std::vector<KF>& v) {
  addKF(v, 800, 0, 0, 0, M_FR | M_FL, 48, 0, F_COOL);
  addKF(v, 1400, 0, 0, 0, M_FR | M_FL, 48, 0);
  addKF(v, 800, 0, 0, 0, 0, 0, 0, F_NEUTRAL);
}
static void gWiggle(std::vector<KF>& v) {
  for (int i = 0; i < 4; i++) {
    addKF(v, 260, +26, 0, 0, 0, 0, 0, F_EXCITED);
    addKF(v, 260, -26, 0, 0, 0, 0, 0);
  }
  addKF(v, 500, 0, 0, 0, 0, 0, 0, F_HAPPY);
}
static void gStretch(std::vector<KF>& v) {
  addKF(v, 900, 0, 0, 0, M_FR | M_FL, 45, 0, F_SLEEPY);   // front low, rear high
  addKF(v, 1200, 0, 18, 0, M_FR | M_FL, 45, 0);
  addKF(v, 900, 0, 0, 0, M_RR | M_RL, 45, 0);
  addKF(v, 1200, 0, -18, 0, M_RR | M_RL, 45, 0);
  addKF(v, 800, 0, 0, 0, 0, 0, 0, F_HAPPY);
}
static void gPushup(std::vector<KF>& v) {
  for (int i = 0; i < 4; i++) {
    addKF(v, 700, 0, 0, 0, M_ALL, 38, 0, F_EXCITED);
    addKF(v, 700, 0, 0, 0, 0, 0, 0);
  }
  addKF(v, 400, 0, 0, 0, 0, 0, 0, F_HAPPY);
}
static void gShy(std::vector<KF>& v) {
  addKF(v, 700, 6, 0, 0, M_ALL, 30, 0, F_SAD);
  addKF(v, 1200, -6, 0, 0, M_ALL, 30, 0);
  addKF(v, 700, 0, 0, 0, M_ALL, 30, 0, F_LOVE);
  addKF(v, 800, 0, 0, 0, 0, 0, 0, F_NEUTRAL);
}
static void gDead(std::vector<KF>& v) {
  addKF(v, 500, 0, 0, 0, M_ALL, 78, 0, F_DIZZY);
  addKF(v, 2800, 0, 0, 0, M_ALL, 78, 0);
  addKF(v, 900, 0, 0, 0, 0, 0, 0, F_SURPRISED);
}
static void gLookAround(std::vector<KF>& v) {   // used by the idle behaviours: a subtle body turn
  addKF(v, 500, +10, 0, 0, 0, 0, 0, F_SUSPICIOUS);
  addKF(v, 700, +10, 0, 0, 0, 0, 0);
  addKF(v, 700, -10, 0, 0, 0, 0, 0);
  addKF(v, 500, 0, 0, 0, 0, 0, 0, F_NEUTRAL);
}

// ---- dances (about 25 s each)
static void dGroove(std::vector<KF>& v) {                    // 120 bpm
  for (int i = 0; i < 8; i++) { addKF(v, 250, 0, 0, 0, M_ALL, 26, 0, i ? 0 : F_EQ); addKF(v, 250, 0, 0, 0, 0, 0, 0); }          // bounce 4 s
  for (int i = 0; i < 8; i++) { addKF(v, 500, +22, 0, 0, 0, 0, 0); addKF(v, 500, -22, 0, 0, 0, 0, 0); }                         // sway 8 s
  for (int i = 0; i < 8; i++) { addKF(v, 500, +12, 0, 0, M_FR | M_RL, 24, 0, i ? 0 : F_HEARTS); addKF(v, 500, -12, 0, 0, M_RR | M_FL, 24, 0); }  // stomps 8 s
  for (int i = 0; i < 4; i++) { addKF(v, 500, 0, +20, 0, 0, 0, 0, i ? 0 : F_EQ); addKF(v, 500, 0, -20, 0, 0, 0, 0); }          // push-pull 4 s
  addKF(v, 600, 0, 0, 0, M_ALL, 30, 0, F_STARS);
  addKF(v, 600, 0, 0, 0, 0, 0, 0);
  addKF(v, 300, 0, 0, 0, 0, 0, 0, F_HAPPY);
}
static void dShuffle(std::vector<KF>& v) {                   // 150 bpm
  for (int i = 0; i < 8; i++) { addKF(v, 400, 0, 0, +18, M_FR | M_RL, 22, 0, i ? 0 : F_SPIRAL); addKF(v, 400, 0, 0, -18, M_RR | M_FL, 22, 0); }   // 6.4 s
  for (int i = 0; i < 8; i++) { addKF(v, 400, 0, +20, 0, 0, 0, 0, i ? 0 : F_NOTES); addKF(v, 400, 0, -20, 0, M_ALL, 20, 0); }                       // 6.4 s
  for (int i = 0; i < 4; i++) { addKF(v, 800, +34, 0, 0, M_FR | M_RL, 20, 0, i ? 0 : F_DIZZY); addKF(v, 800, -34, 0, 0, M_RR | M_FL, 20, 0); }    // 6.4 s
  for (int i = 0; i < 16; i++) { addKF(v, 200, +15, 0, 0, 0, 0, 0, i ? 0 : F_STARS); addKF(v, 200, -15, 0, 0, M_ALL, 18, 0); }                    // 6.4 s
  addKF(v, 500, 0, 0, 0, 0, 0, 0, F_HAPPY);
}
static void dRobot(std::vector<KF>& v) {                     // snap and hold
  static const struct { int8_t tw, fb, dg; uint8_t mask; } P[8] = {
    {20, 0, 0, 0}, {-20, 0, 0, 0}, {0, 18, 0, 0}, {0, -18, 0, 0},
    {0, 0, 0, M_ALL}, {0, 0, 0, M_FR | M_RL}, {0, 0, 0, M_RR | M_FL}, {20, 14, 0, M_ALL}};
  for (int r = 0; r < 5; r++)
    for (int i = 0; i < 8; i++) {
      uint8_t f = (r == 0 && i == 0) ? F_GLITCH : 0;
      addKF(v, 150, P[i].tw, P[i].fb, P[i].dg, P[i].mask, 28, 0, f);
      addKF(v, 450, P[i].tw, P[i].fb, P[i].dg, P[i].mask, 28, 0);
    }
  addKF(v, 600, 0, 0, 0, 0, 0, 0, F_COOL);
}
static void dRipple(std::vector<KF>& v) {                    // 100 bpm, a wave of lifted legs around the body
  static const uint8_t ORDER[4] = {M_FR, M_RR, M_RL, M_FL};
  for (int r = 0; r < 10; r++) {
    float tw = 20 * sinf(r * 0.8f);
    for (int j = 0; j < 4; j++) {
      addKF(v, 300, tw, 0, 0, ORDER[j], 32, 0, (r == 0 && j == 0) ? F_MATRIX : (j == 0 && r == 5 ? F_HEARTS : 0));
      addKF(v, 300, tw, 0, 0, 0, 0, 0);
    }
  }
  addKF(v, 500, 0, 0, 0, 0, 0, 0, F_HAPPY);
}

struct NamedAnim { const char* name; void (*build)(std::vector<KF>&); bool dance; };
static const NamedAnim ANIMS[] = {
  {"wave", gWave, false}, {"bow", gBow, false}, {"wiggle", gWiggle, false}, {"stretch", gStretch, false},
  {"pushup", gPushup, false}, {"shy", gShy, false}, {"dead", gDead, false}, {"look", gLookAround, false},
  {"groove", dGroove, true}, {"shuffle", dShuffle, true}, {"robot", dRobot, true}, {"ripple", dRipple, true},
};
static const int ANIM_N = sizeof(ANIMS) / sizeof(ANIMS[0]);

static const NamedAnim* findAnim(const String& n) {
  for (int i = 0; i < ANIM_N; i++) if (n == ANIMS[i].name) return &ANIMS[i];
  return nullptr;
}

// ---- user dances: one keyframe per line "ms s0 s1 s2 s3 a0 a1 a2 a3 face" (leg order FR RR FL RL)
static bool parseDance(const String& text, std::vector<KF>& v) {
  int pos = 0, n = text.length();
  while (pos < n && v.size() < 400) {
    int e = text.indexOf('\n', pos);
    if (e < 0) e = n;
    String line = text.substring(pos, e);
    pos = e + 1;
    line.trim();
    if (!line.length() || line[0] == '#') continue;
    int ms = 0, face = 0;
    float s[4], a[4];
    int got = sscanf(line.c_str(), "%d %f %f %f %f %f %f %f %f %d", &ms, &s[0], &s[1], &s[2], &s[3], &a[0], &a[1], &a[2], &a[3], &face);
    if (got < 9) return false;
    KF k;
    k.ms = constrain(ms, 0, 20000);
    k.face = constrain(face, 0, 255);
    for (int i = 0; i < 4; i++) { k.s[i] = constrain(s[i], -MAX_SWING, MAX_SWING); k.a[i] = constrain(a[i], 0.0f, MAX_SPLAY); }
    v.push_back(k);
  }
  return !v.empty();
}

static String cleanName(String n) {
  String o;
  for (unsigned i = 0; i < n.length() && o.length() < 16; i++) {
    char c = n[i];
    if (isalnum(c) || c == '_' || c == '-') o += c;
  }
  return o;
}
