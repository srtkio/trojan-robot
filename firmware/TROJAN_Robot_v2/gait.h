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
//   stride_i = S * (vx + LATSIGN_i * vy + ROTSIGN_i * wz)
// LATSIGN: sign of the sideways component of a forward hip swing (+ for FR/RL, - for RR/FL).
// ROTSIGN: right-hand legs push the body round counter-clockwise, left-hand legs clockwise.
//
// Modes (duty = share of the cycle a foot is on the ground):
//   0 crawl  one leg in the air at a time (FR, RL, FL, RR), duty 0.75: always 3 feet down, slow, very stable
//   1 trot   diagonal pairs (FR+RL, then RR+FL), duty 0.55: 2 feet down, quicker, the 1.0 default for speed
//   2 run    trot timing with duty 0.50, 1.7x cadence, longer stride, lower lift: the fast dog run
// Crawl and trot are bit-for-bit the 1.0 gait.
enum : uint8_t { GM_CRAWL, GM_TROT, GM_RUN, GM_N };

struct Gait {
  float phase = 0;
  float vx = 0, vy = 0, wz = 0;  // smoothed command
  float mag = 0;
  float ofs[LEG_N] = {0, 0, 0, 0};
  float duty = 0.75f;
  bool init = false;
  Pose out = makePose();

  void reset() { vx = vy = wz = mag = 0; }

  void update(float dt, float cx, float cy, float cw, const Cfg& c, uint8_t mode) {
    static const float LATSIGN[LEG_N] = {+1, -1, -1, +1};
    static const float ROTSIGN[LEG_N] = {+1, +1, -1, -1};
    static const float OFS[GM_N][LEG_N] = {     // leg order FR RR FL RL
      {0.00f, 0.75f, 0.50f, 0.25f},
      {0.00f, 0.50f, 0.50f, 0.00f},
      {0.00f, 0.50f, 0.50f, 0.00f}};
    static const float DUTY[GM_N] = {0.75f, 0.55f, 0.50f};
    static const float FREQK[GM_N] = {1.0f, 1.0f, 1.7f};
    static const float STRIDEK[GM_N] = {1.0f, 1.0f, 1.15f};
    static const float LIFTK[GM_N] = {1.0f, 1.0f, 0.8f};
    uint8_t m = mode < GM_N ? mode : GM_CRAWL;

    float k = min(1.0f, dt / 0.10f);
    vx += (cx - vx) * k;
    vy += (cy - vy) * k;
    wz += (cw - wz) * k;
    mag = max(max(fabsf(vx), fabsf(vy)), fabsf(wz));
    if (mag > 0.02f) phase += min(c.freq * FREQK[m], 4.2f) * dt;
    if (phase > 1000) phase -= 1000;

    // A mode change mid-walk slides the leg timing over ~0.3 s instead of teleporting legs.
    if (!init) { for (int i = 0; i < LEG_N; i++) ofs[i] = OFS[m][i]; duty = DUTY[m]; init = true; }
    float kb = min(1.0f, dt / 0.3f);
    for (int i = 0; i < LEG_N; i++) {
      float d = OFS[m][i] - ofs[i];
      d -= roundf(d);                       // shortest way round the cycle
      ofs[i] += d * kb;
      ofs[i] -= floorf(ofs[i]);
    }
    duty += (DUTY[m] - duty) * kb;

    float w = wz + c.steer * vx;
    float liftScale = min(1.0f, mag * 5.0f);

    for (int i = 0; i < LEG_N; i++) {
      float a = c.stride * STRIDEK[m] * c.trimStride[i] * (vx + LATSIGN[i] * vy + ROTSIGN[i] * w);
      a = constrain(a, -MAX_SWING + 4, MAX_SWING - 4);
      float th = phase - ofs[i];
      th -= floorf(th);
      float s, l = 0;
      if (th < duty) {
        s = a * (1 - 2 * th / duty);                      // stance: sweep +a -> -a
      } else {
        float v = (th - duty) / (1 - duty);
        s = -a + 2 * a * (0.5f - 0.5f * cosf(PI * v));    // swing: -a -> +a, eased
        l = c.lift * LIFTK[m] * c.trimLift[i] * sinf(PI * v) * liftScale;
      }
      out.s[i] = s + c.trimHip[i];
      out.a[i] = c.height + l;
    }
  }
};
