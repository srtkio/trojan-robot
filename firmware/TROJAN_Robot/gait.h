#pragma once
// Settings (persisted) + the walking engine.
#include <Preferences.h>
#include "robot.h"

struct Cfg {
  uint32_t magic = 0x544A0002;
  uint8_t mode = 0;            // 0 crawl (3 feet always down), 1 trot (diagonal pairs)
  float freq = 1.0f;           // gait cycles per second
  float stride = 20;           // hip half-swing at full stick (deg)
  float lift = 24;             // foot splay while a leg is in the air (deg)
  float height = 0;            // base foot splay while standing/walking: 0 = tall, larger = lower + wider
  float steer = 0;             // constant turn bias, cancels drift (-0.5..0.5)
  float trimStride[LEG_N] = {1, 1, 1, 1};  // per-leg stride scale (rubber grips bite differently)
  float trimHip[LEG_N] = {0, 0, 0, 0};     // per-leg hip neutral offset (deg)
  float trimLift[LEG_N] = {1, 1, 1, 1};    // per-leg lift scale
  int8_t servoTrim[SERVO_N] = {0, 0, 0, 0, 0, 0, 0, 0};  // raw-angle offsets
  float maxSpeed = 420;        // servo slew limit (deg/s)
  uint16_t deadMs = 500;       // drive commands older than this are ignored (dead-man)
  uint8_t life = 1;            // idle behaviours (looking around, stretching, yawning)
  uint8_t react = 1;          // reactions (greeting a new connection, etc.)
  uint16_t sleepSec = 180;     // seconds of inactivity before TROJAN goes to sleep
  uint16_t relaxSec = 8;       // seconds seated before servos are released (silences buzzing, saves power)
};

inline bool cfgLoad(Cfg& c) {
  Preferences p;
  if (!p.begin("trojan", true)) return false;
  Cfg tmp;
  bool ok = p.getBytesLength("cfg") == sizeof(Cfg) && p.getBytes("cfg", &tmp, sizeof(Cfg)) == sizeof(Cfg) && tmp.magic == c.magic;
  p.end();
  if (ok) c = tmp;
  return ok;
}

inline void cfgSave(const Cfg& c) {
  Preferences p;
  if (!p.begin("trojan", false)) return;
  p.putBytes("cfg", &c, sizeof(Cfg));
  p.end();
}

// Walking: each leg runs its own phase. Stance = foot down, hip sweeps backward at a constant rate
// (so no foot is dragged); swing = foot lifts, hip carries it forward. Direction comes from the
// stick via three per-leg coefficients derived from the leg geometry:
//   stride_i = S * (vx + EPS_i * vy + SIG_i * wz)
// LATSIGN: sign of the sideways component of a forward hip swing (+ for FR/RL, - for RR/FL).
// ROTSIGN: right-hand legs push the body round counter-clockwise, left-hand legs clockwise.
struct Gait {
  float phase = 0;
  float vx = 0, vy = 0, wz = 0;  // smoothed command
  float mag = 0;
  Pose out = makePose();

  void reset() { vx = vy = wz = mag = 0; }

  void update(float dt, float cx, float cy, float cw, const Cfg& c) {
    static const float LATSIGN[LEG_N] = {+1, -1, -1, +1};
    static const float ROTSIGN[LEG_N] = {+1, +1, -1, -1};
    static const float OFS_CRAWL[LEG_N] = {0.00f, 0.75f, 0.50f, 0.25f};  // FR, RL, FL, RR order
    static const float OFS_TROT[LEG_N] = {0.00f, 0.50f, 0.50f, 0.00f};   // FR+RL, then RR+FL

    float k = min(1.0f, dt / 0.10f);
    vx += (cx - vx) * k;
    vy += (cy - vy) * k;
    wz += (cw - wz) * k;
    mag = max(max(fabsf(vx), fabsf(vy)), fabsf(wz));
    if (mag > 0.02f) phase += c.freq * dt;
    if (phase > 1000) phase -= 1000;

    const float duty = c.mode ? 0.55f : 0.75f;
    const float* ofs = c.mode ? OFS_TROT : OFS_CRAWL;
    float w = wz + c.steer * vx;
    float liftScale = min(1.0f, mag * 5.0f);

    for (int i = 0; i < LEG_N; i++) {
      float a = c.stride * c.trimStride[i] * (vx + LATSIGN[i] * vy + ROTSIGN[i] * w);
      a = constrain(a, -MAX_SWING + 4, MAX_SWING - 4);
      float th = phase - ofs[i];
      th -= floorf(th);
      float s, l = 0;
      if (th < duty) {
        s = a * (1 - 2 * th / duty);                      // stance: sweep +a -> -a
      } else {
        float v = (th - duty) / (1 - duty);
        s = -a + 2 * a * (0.5f - 0.5f * cosf(PI * v));    // swing: -a -> +a, eased
        l = c.lift * c.trimLift[i] * sinf(PI * v) * liftScale;
      }
      out.s[i] = s + c.trimHip[i];
      out.a[i] = c.height + l;
    }
  }
};
