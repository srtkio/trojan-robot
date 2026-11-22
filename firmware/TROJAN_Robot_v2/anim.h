#pragma once
// Motion library v2. Two players:
//  * Moves: every built-in trick and dance is a continuous function of time, evaluated every 20 ms.
//    Nothing steps through keyframes, so there are no stop-start jerks; amplitudes use the full range,
//    because micro servos look smooth on big brisk moves and twitchy on tiny slow ones.
//  * Keyframes: Pose Lab dances and the boot sequence. Interpolated with a monotone cubic, so the legs
//    flow THROUGH each keyframe instead of stopping, never overshoot a pose, and holds stay still.
#include <vector>
#include <LittleFS.h>
#include "robot.h"

// Face ids used by the moves (kept in sync with face.h).
enum : uint8_t {
  F_NEUTRAL = 1, F_HAPPY, F_SAD, F_ANGRY, F_SURPRISED, F_SLEEPY, F_LOVE, F_EXCITED, F_WINK, F_DIZZY, F_SUSPICIOUS, F_COOL,
  F_CURIOUS, F_SCARED, F_BORED, F_LAUGH, F_DETERMINED,
  F_EQ = 32, F_HEARTS, F_STARS, F_SPIRAL, F_GLITCH, F_NOTES, F_MATRIX, F_TITLE, F_ECG
};

// ================================================================ wave toolkit
// Leg order everywhere: FR RR FL RL.
static const float TWS[LEG_N] = {+1, +1, -1, -1};      // body twist: + turns the body counter-clockwise
static const float FRONT[LEG_N] = {+1, -1, +1, -1};    // pitch: + = nose down
static const float RIGHT[LEG_N] = {+1, +1, -1, -1};    // roll: + = right side down
static const float CORNER[LEG_N] = {0.785f, -0.785f, 2.356f, -2.356f};  // corner angle round the body

static inline float ease(float u) { u = constrain(u, 0.0f, 1.0f); return u * u * (3 - 2 * u); }
static inline float sn(float x) { return sinf(2 * PI * x); }   // one cycle per unit of x
static inline float cs(float x) { return cosf(2 * PI * x); }
static inline float pos(float v) { return v > 0 ? v : 0; }
// Squarer than a sine (snappy hits) but with a finite slope everywhere, so it never jumps.
static inline float punch(float x) { return tanhf(2.2f * sn(x)) / 0.9757f; }
// 0 outside [x0,x1], eased up/down over r at the edges.
static inline float win(float x, float x0, float x1, float r) { return ease((x - x0) / r) * ease((x1 - x) / r); }

static inline void body(Pose& p, float base, float tw, float fb, float pitch, float roll) {
  for (int i = 0; i < LEG_N; i++) {
    p.s[i] += tw * TWS[i] + fb;
    p.a[i] += base + pitch * FRONT[i] + roll * RIGHT[i];
  }
}
static inline void addPose(Pose& p, const Pose& q, float w) {
  for (int i = 0; i < LEG_N; i++) { p.s[i] += q.s[i] * w; p.a[i] += q.a[i] * w; }
}
static inline void liftPair(Pose& p, float diagA, float diagB) {   // A = FR+RL, B = RR+FL
  p.a[FR] += diagA; p.a[RL] += diagA; p.a[RR] += diagB; p.a[FL] += diagB;
}
// Counterweight for lifting ONE leg (tuned on TROJAN after Hi tipped onto its nose): the diagonal corner
// sinks most, the same-side corner a bit, and the two neighbours step toward the lifted corner so the
// support triangle grows under the body. k = 0..1.
static inline void brace(Pose& p, uint8_t L, float k) {
  float dir = FRONT[L];
  p.a[L ^ 3] += 44 * k;              // diagonal
  p.a[L ^ 1] += 30 * k;              // same side, other end
  p.s[L ^ 2] += 14 * dir * k;        // other leg at the same end
  p.s[L ^ 1] += 14 * dir * k;
}

// ================================================================ moves
// fn(t seconds, b beats, pose to add into, face to request). Poses are around 0 (standing tall).
typedef void (*MoveFn)(float t, float b, Pose& p, uint8_t& f);
struct Move { const char* name; const char* label; MoveFn fn; float beats; float bpm; uint8_t kind; };  // kind 0 trick, 1 dance

// ---- tricks (bpm 60, so beats = seconds)
static void mHi(float t, float, Pose& p, uint8_t& f) {
  float br = ease(t / 0.7f) * ease((5.6f - t) / 0.6f);    // brace first, release last
  float paw = win(t, 0.7f, 4.8f, 0.55f), wv = win(t, 1.2f, 4.4f, 0.4f);
  brace(p, FR, br);
  p.s[FR] += 10 * paw + 28 * wv * sn(1.6f * (t - 1.2f));  // big side-to-side wave with the paw held high
  p.a[FR] += 110 * paw;
  f = (t < 1.2f || t > 4.4f) ? F_HAPPY : F_EXCITED;
}
static void mPaw(float t, float, Pose& p, uint8_t& f) {   // shake hands
  float br = ease(t / 0.7f) * ease((4.8f - t) / 0.6f);
  float paw = win(t, 0.7f, 4.0f, 0.5f), wv = win(t, 1.2f, 3.6f, 0.3f);
  brace(p, FR, br);
  p.s[FR] += 30 * paw;
  p.a[FR] += 72 * paw + 26 * wv * sn(1.5f * (t - 1.2f));
  f = F_LOVE;
}
static void mNod(float t, float, Pose& p, uint8_t& f) {
  float v = 32 * win(t, 0, 2.6f, 0.3f) * 0.5f * (1 - cs(1.5f * t));
  p.a[FR] += v; p.a[FL] += v;
  f = F_HAPPY;
}
static void mShake(float t, float, Pose& p, uint8_t& f) {
  body(p, 0, 28 * win(t, 0, 2.6f, 0.3f) * sn(2.0f * t), 0, 0, 0);
  f = F_SUSPICIOUS;
}
static void mBow(float t, float, Pose& p, uint8_t& f) {    // play bow + tail wag
  float w = win(t, 0, 3.8f, 0.6f), wag = 16 * win(t, 0.8f, 3.2f, 0.3f) * sn(2.5f * t);
  p.a[FR] += 62 * w; p.a[FL] += 62 * w;
  p.s[FR] += 20 * w; p.s[FL] += 20 * w;
  p.s[RR] += wag; p.s[RL] -= wag;
  f = F_EXCITED;
}
static void mDogSit(float t, float, Pose& p, uint8_t& f) {
  float w = win(t, 0, 4.4f, 0.8f), tilt = win(t, 1.4f, 3.2f, 0.3f);
  p.a[RR] += 58 * w; p.a[RL] += 58 * w;
  p.s[FR] += 12 * w; p.s[FL] += 12 * w;
  p.a[FR] += 14 * tilt;                                     // head tilt
  f = t > 1.2f ? F_CURIOUS : F_HAPPY;
}
static void mStretch(float t, float, Pose& p, uint8_t& f) {
  float a1 = win(t, 0, 2.4f, 0.7f), a2 = win(t, 2.6f, 5.0f, 0.7f);
  p.a[FR] += 60 * a1; p.a[FL] += 60 * a1; p.s[FR] += 30 * a1; p.s[FL] += 30 * a1;   // front reach
  p.a[RR] += 55 * a2; p.a[RL] += 55 * a2; p.s[RR] -= 30 * a2; p.s[RL] -= 30 * a2;   // back legs
  f = t < 2.5f ? F_SLEEPY : F_HAPPY;
}
static void mPushup(float t, float, Pose& p, uint8_t& f) {
  body(p, 64 * win(t, 0, 5.0f, 0.4f) * 0.5f * (1 - cs(0.8f * t)), 0, 0, 0, 0);
  f = F_DETERMINED;
}
static void mWiggle(float t, float, Pose& p, uint8_t& f) {   // snake: rear follows the front
  float w = win(t, 0, 3.6f, 0.5f), fr = 34 * w * sn(1.2f * t), rr = 34 * w * sn(1.2f * t - 0.2f);
  p.s[FR] += fr; p.s[FL] -= fr; p.s[RR] += rr; p.s[RL] -= rr;
  body(p, 8 * w * (1 - cs(2.4f * t)), 0, 0, 0, 0);
  f = F_EXCITED;
}
static void mHula(float t, float, Pose& p, uint8_t& f) {     // low corner circles the body
  float w = win(t, 0, 5.0f, 0.8f), ph = 0.8f * t;
  body(p, 30 * w, 18 * w * sn(ph), 0, 20 * w * cs(ph), 20 * w * sn(ph));
  f = F_LOVE;
}
static void mTap(float t, float, Pose& p, uint8_t& f) {
  brace(p, FR, 0.7f * win(t, 0, 3.4f, 0.5f));
  p.a[FR] += (8 + 26 * pos(sn(2.4f * (t - 0.5f)))) * win(t, 0.5f, 3.0f, 0.3f);
  f = F_BORED;
}
static void mPee(float t, float, Pose& p, uint8_t& f) {
  float up = win(t, 0.8f, 3.6f, 0.5f);
  brace(p, RL, win(t, 0, 4.4f, 0.7f));
  p.a[RL] += 105 * up; p.s[RL] -= 12 * up;
  f = t < 1.0f ? F_SUSPICIOUS : F_WINK;
}
static void mScratch(float t, float, Pose& p, uint8_t& f) {
  float up = win(t, 0.7f, 3.3f, 0.4f);
  brace(p, RR, win(t, 0, 4.0f, 0.7f));
  p.a[RR] += 95 * up;
  p.s[RR] += up * (-6 + 14 * sn(5.0f * (t - 0.7f)));      // fast scratching
  f = F_LAUGH;
}
static void mSneeze(float t, float, Pose& p, uint8_t& f) {
  float wind = ease(t / 1.3f) * ease((1.5f - t) / 0.1f);
  float snap = ease((t - 1.4f) / 0.12f) * ease((2.8f - t) / 0.5f);
  p.a[RR] += 34 * wind; p.a[RL] += 34 * wind;               // head goes up...
  p.a[FR] += 50 * snap; p.a[FL] += 50 * snap;               // ...achoo
  body(p, 0, 0, -8 * snap, 0, 0);
  f = t < 1.4f ? F_SURPRISED : F_LAUGH;
}
static void mShiver(float t, float, Pose& p, uint8_t& f) {
  float w = win(t, 0, 2.2f, 0.3f);
  body(p, w * (8 + 4 * sn(9 * t)), 7 * w * sn(7 * t), 0, 0, 0);
  f = F_SCARED;
}
static void mPounce(float t, float, Pose& p, uint8_t& f) {
  float crouch = ease(t / 0.8f) * ease((1.6f - t) / 0.15f);
  float lunge = ease((t - 1.45f) / 0.15f) * ease((3.6f - t) / 0.8f);
  p.a[RR] += 45 * crouch; p.a[RL] += 45 * crouch; p.a[FR] += 20 * crouch; p.a[FL] += 20 * crouch;
  body(p, 0, 0, 15 * crouch - 35 * lunge, 0, 0);
  f = t < 1.45f ? F_SUSPICIOUS : t < 2.4f ? F_DETERMINED : F_EXCITED;
}
static void mDead(float t, float, Pose& p, uint8_t& f) {     // belly flop, legs in the air
  body(p, 110 * ease(t / 0.6f) * ease((5.4f - t) / 0.9f), 0, 0, 0, 0);
  f = t < 1.2f ? F_DIZZY : t < 4.4f ? F_ECG : F_SURPRISED;
}
static void mLook(float t, float, Pose& p, uint8_t& f) {
  body(p, 0, 16 * win(t, 0, 3.4f, 0.4f) * sn(t / 3.4f), 0, 0, 0);
  f = F_SUSPICIOUS;
}

// ---- dances: sections on the beat, cross-faded over one beat so a dance never stops between parts
typedef void (*SecFn)(float x, Pose& q);
struct Sec { float b0, b1; SecFn fn; uint8_t face; };
static void runSecs(const Sec* S, int n, float b, Pose& p, uint8_t& f) {
  for (int k = 0; k < n; k++) {
    if (b >= S[k].b0 && b < S[k].b1) f = S[k].face;
    float w = win(b, S[k].b0 - 0.5f, S[k].b1 + 0.5f, 1.0f);
    if (w <= 0) continue;
    Pose q = makePose();
    S[k].fn(b - S[k].b0, q);
    addPose(p, q, w);
  }
}
#define DANCE(name, secs) static void name(float, float b, Pose& p, uint8_t& f) { runSecs(secs, sizeof(secs) / sizeof(Sec), b, p, f); }

// Groove 120 bpm
static void gBounce(float x, Pose& q) { body(q, 27 - 20 * cs(x), 10 * sn(x / 2), 0, 0, 7 * sn(x / 2)); }
static void gHula(float x, Pose& q) { float ph = x / 2; body(q, 30, 22 * sn(ph), 0, 20 * cs(ph), 20 * sn(ph)); }
static void gStomp(float x, Pose& q) { float u = sn(x / 2); liftPair(q, 45 * pos(u), 45 * pos(-u)); body(q, 0, 28 * u, 0, 0, 0); }
static void gPush(float x, Pose& q) { body(q, 22, 0, 32 * punch(x / 2), 18 * sn(x / 2 + 0.25f), 0); }
static const Sec GROOVE[] = {{0, 8, gBounce, F_EQ}, {8, 16, gHula, F_HEARTS}, {16, 24, gStomp, F_NOTES}, {24, 32, gPush, F_STARS}};
DANCE(dGroove, GROOVE)

// Shuffle 150 bpm
static void sRun(float x, Pose& q) { float u = sn(x / 2); liftPair(q, 40 * pos(u), 40 * pos(-u)); body(q, 0, 0, 18 * cs(x / 2), 0, 0); }
static void sSlide(float x, Pose& q) { body(q, 26, -16 * punch(x / 2), 0, 0, 22 * punch(x / 2)); }
static void sTwirl(float x, Pose& q) { float u = sn(x); liftPair(q, 30 * pos(u), 30 * pos(-u)); body(q, 0, 35 * sn(x / 4), 0, 0, 0); }
static const Sec SHUFFLE[] = {{0, 16, sRun, F_NOTES}, {16, 24, sSlide, F_EQ}, {24, 32, sTwirl, F_STARS}};
DANCE(dShuffle, SHUFFLE)

// Robot 110 bpm: hard snaps on every beat, holds in between (on purpose)
static void rSnap(float x, Pose& q) {
  static const float P[8][6] = {   // twist, surge, pitch, roll, pair (1 = FR+RL, 2 = RR+FL), lift
    {30, 0, 0, 0, 0, 0}, {-30, 0, 0, 0, 0, 0}, {0, 30, 0, 0, 0, 0}, {0, -30, 0, 0, 0, 0},
    {0, 0, 18, 0, 0, 0}, {0, 0, -18, 0, 0, 0}, {0, 0, 0, 0, 1, 45}, {25, 20, 0, 0, 2, 45}};
  int k = (int)floorf(x);
  const float* A = P[(k + 7) % 8];
  const float* B = P[k % 8];
  float u = ease((x - k) / 0.18f);
  float v[6];
  for (int i = 0; i < 6; i++) v[i] = A[i] + (B[i] - A[i]) * u;
  body(q, 20, v[0], v[1], v[2], v[3]);
  float la = (A[4] == 1 ? A[5] * (1 - u) : 0) + (B[4] == 1 ? B[5] * u : 0);
  float lb = (A[4] == 2 ? A[5] * (1 - u) : 0) + (B[4] == 2 ? B[5] * u : 0);
  liftPair(q, la, lb);
}
static const Sec ROBOT[] = {{0, 32, rSnap, F_GLITCH}};
DANCE(dRobot, ROBOT)

// Ripple 100 bpm: one leg up at a time, travelling round the body, with the opposite corner leaning away
static void rRipple(float x, Pose& q) {
  float ph = x / 4;
  for (int i = 0; i < LEG_N; i++) {
    float c = cosf(2 * PI * ph - CORNER[i]);
    float up = pos(c);
    q.a[i] += 55 * up * up * up + 40 * 0.5f * (1 - c);
  }
  body(q, 0, 14 * sn(ph), 12 * cosf(2 * PI * ph), 0, 0);
}
static const Sec RIPPLE[] = {{0, 12, rRipple, F_MATRIX}, {12, 24, rRipple, F_HEARTS}};
DANCE(dRipple, RIPPLE)

// Flow 96 bpm: liquid, nothing but waves
static void fHula(float x, Pose& q) { float ph = x / 4; body(q, 30, 20 * sn(ph), 0, 21 * cs(ph), 21 * sn(ph)); }
static void fSnake(float x, Pose& q) {
  float fr = 34 * sn(x / 2), rr = 34 * sn(x / 2 - 0.2f);
  q.s[FR] += fr; q.s[FL] -= fr; q.s[RR] += rr; q.s[RL] -= rr;
  body(q, 10 * (1 - cs(x)), 0, 0, 0, 0);
}
static void fSurf(float x, Pose& q) {
  float fr = 32 * (0.5f + 0.5f * sn(x / 2)), rr = 32 * (0.5f + 0.5f * sn(x / 2 - 0.3f));
  q.a[FR] += fr; q.a[FL] += fr; q.a[RR] += rr; q.a[RL] += rr;
  body(q, 0, 0, 22 * cs(x / 2), 0, 0);
}
static const Sec FLOW[] = {{0, 10, fHula, F_MATRIX}, {10, 20, fSnake, F_EQ}, {20, 32, fSurf, F_STARS}};
DANCE(dFlow, FLOW)

// Twerk 128 bpm: nose down, tail bouncing twice a beat
static void tTwerk(float x, Pose& q, float wag) {
  q.a[FR] += 42; q.a[FL] += 42; q.s[FR] += 22; q.s[FL] += 22;
  float r = 28 * 0.5f * (1 - cs(2 * x));
  q.a[RR] += r; q.a[RL] += r;
  q.s[RR] += wag * sn(x); q.s[RL] -= wag * sn(x);
}
static void tw1(float x, Pose& q) { tTwerk(x, q, 14); }
static void tw2(float x, Pose& q) { tTwerk(x, q, 24); }
static const Sec TWERK[] = {{0, 12, tw1, F_COOL}, {12, 24, tw2, F_LAUGH}};
DANCE(dTwerk, TWERK)

// Headbang 140 bpm: nose hits on the beat, then left/right alternating
static void hBang(float x, Pose& q) { float v = 24 + 24 * cs(x); q.a[FR] += v; q.a[FL] += v; q.a[RR] += 4; q.a[RL] += 4; q.s[RR] -= 10; q.s[RL] -= 10; }
static void hSide(float x, Pose& q) { q.a[FR] += 48 * pos(cs(x / 2)); q.a[FL] += 48 * pos(-cs(x / 2)); q.s[RR] -= 10; q.s[RL] -= 10; }
static const Sec HEADBANG[] = {{0, 12, hBang, F_GLITCH}, {12, 24, hSide, F_EQ}};
DANCE(dHeadbang, HEADBANG)

// Disco 115 bpm: point one front paw to the sky, then the other (each one fully braced)
static void dPoint(float x, Pose& q) {
  float kr = ease(1.6f * pos(sn(x / 4))), kl = ease(1.6f * pos(-sn(x / 4)));
  brace(q, FR, kr); brace(q, FL, kl);
  q.a[FR] += 100 * kr; q.s[FR] += 36 * kr;
  q.a[FL] += 100 * kl; q.s[FL] += 36 * kl;
  body(q, 6 * pos(-cs(x)), 0, 0, 0, 0);
}
static const Sec DISCO[] = {{0, 16, dPoint, F_STARS}, {16, 32, dPoint, F_COOL}};
DANCE(dDisco, DISCO)

static const Move MOVES[] = {
  {"wave", "Hi", mHi, 5.6f, 60, 0},          {"paw", "Give paw", mPaw, 4.8f, 60, 0},
  {"nod", "Yes", mNod, 2.6f, 60, 0},         {"shake", "No", mShake, 2.6f, 60, 0},
  {"bow", "Play bow", mBow, 3.8f, 60, 0},    {"dogsit", "Sit pretty", mDogSit, 4.4f, 60, 0},
  {"stretch", "Stretch", mStretch, 5.2f, 60, 0}, {"pushup", "Push-ups", mPushup, 5.0f, 60, 0},
  {"wiggle", "Wiggle", mWiggle, 3.6f, 60, 0}, {"hula", "Hula", mHula, 5.0f, 60, 0},
  {"tap", "Tap", mTap, 3.4f, 60, 0},         {"pee", "Pee", mPee, 4.4f, 60, 0},
  {"scratch", "Scratch", mScratch, 4.0f, 60, 0}, {"sneeze", "Sneeze", mSneeze, 2.8f, 60, 0},
  {"shiver", "Shiver", mShiver, 2.2f, 60, 0}, {"pounce", "Pounce", mPounce, 3.6f, 60, 0},
  {"dead", "Play dead", mDead, 5.4f, 60, 0}, {"look", "Look around", mLook, 3.4f, 60, 0},
  {"groove", "Groove", dGroove, 32, 120, 1}, {"shuffle", "Shuffle", dShuffle, 32, 150, 1},
  {"disco", "Disco", dDisco, 32, 115, 1},    {"twerk", "Twerk", dTwerk, 24, 128, 1},
  {"headbang", "Headbang", dHeadbang, 24, 140, 1}, {"flow", "Flow", dFlow, 32, 96, 1},
  {"ripple", "Ripple", dRipple, 24, 100, 1}, {"robot", "Robot", dRobot, 32, 110, 1},
};
static const int MOVE_N = sizeof(MOVES) / sizeof(MOVES[0]);
static const Move* findMove(const String& n) {
  for (int i = 0; i < MOVE_N; i++) if (n == MOVES[i].name) return &MOVES[i];
  return nullptr;
}

// ================================================================ keyframes (Pose Lab dances, boot)
struct KF {
  uint16_t ms;
  float s[LEG_N];
  float a[LEG_N];
  uint8_t face;  // 0 = leave the face alone, else a face id
};

#define M_FR 1
#define M_RR 2
#define M_FL 4
#define M_RL 8
#define M_ALL 15

static void addKF(std::vector<KF>& v, uint16_t ms, float tw, float fb, float dg, uint8_t mask, float lift, float base, uint8_t face = 0) {
  static const float DG[LEG_N] = {+1, -1, -1, +1};
  KF k;
  k.ms = ms;
  k.face = face;
  for (int i = 0; i < LEG_N; i++) {
    k.s[i] = constrain(tw * TWS[i] + fb + dg * DG[i], -MAX_SWING, MAX_SWING);
    k.a[i] = constrain(base + (((mask >> i) & 1) ? lift : 0), 0.0f, MAX_SPLAY);
  }
  v.push_back(k);
}

// Monotone cubic slope at a keyframe from the slopes either side (value per ms). Zero at a turn-around
// or flat spot (so holds stay still), harmonic mean otherwise (so the curve never overshoots).
static float mslope(float d0, float t0, float d1, float t1) {
  if (t0 <= 0 || t1 <= 0) return 0;
  float a = d0 / t0, b = d1 / t1;
  if (a * b <= 0) return 0;
  return 2 * a * b / (a + b);
}
static float herm(float pp, float p0, float p1, float pn, float tp, float T, float tn, float u) {
  float m0 = mslope(p0 - pp, tp, p1 - p0, T), m1 = mslope(p1 - p0, T, pn - p1, tn);
  float u2 = u * u, u3 = u2 * u;
  return (2 * u3 - 3 * u2 + 1) * p0 + (u3 - 2 * u2 + u) * T * m0 + (-2 * u3 + 3 * u2) * p1 + (u3 - u2) * T * m1;
}

// ================================================================ player
struct Anim {
  bool active = false;
  bool cancellable = true;   // a joystick push stops it
  String name;
  void (*onFace)(uint8_t) = nullptr;
  void (*onTitle)(const char*) = nullptr;
  Pose from;
  // move mode
  const Move* mv = nullptr;
  float t = 0, dur = 0, rate = 1, blendIn = 0.35f, restA = 0;
  uint8_t wantFace = 0, shownFace = 0;
  bool titled = false;
  // keyframe mode
  std::vector<KF> kf;
  size_t idx = 0;
  uint32_t elapsed = 0, prevMs = 0;
  Pose prev;

  void startMove(const Move* m, const Pose& now, float tempo, float restHeight) {
    kf.clear();
    mv = m;
    name = m->name;
    from = now;
    t = 0;
    rate = m->kind == 1 ? constrain(tempo, 0.5f, 1.8f) : 1.0f;
    dur = m->beats * 60.0f / (m->bpm * rate);
    restA = restHeight;
    float far = 0;                                   // rising from a sit takes longer than from a stand
    for (int i = 0; i < LEG_N; i++) far = max(far, max(fabsf(now.s[i]), fabsf(now.a[i])));
    blendIn = constrain(far / 100.0f, 0.35f, 1.0f);
    wantFace = shownFace = 0;
    titled = m->kind == 1;
    if (titled && onTitle) onTitle(m->label);
    active = true;
  }
  void startKF(const String& n, const std::vector<KF>& frames, const Pose& now) {
    mv = nullptr;
    kf = frames;
    name = n;
    idx = 0;
    elapsed = 0;
    prevMs = 0;
    from = prev = now;
    active = !kf.empty();
    if (active) fireFace();
  }
  void stop() { active = false; name = ""; mv = nullptr; }
  void fireFace() { if (onFace && kf[idx].face) onFace(kf[idx].face); }

  bool tick(uint32_t dtMs, Pose& out) {
    if (!active) return false;
    return mv ? tickMove(dtMs, out) : tickKF(dtMs, out);
  }

  bool tickMove(uint32_t dtMs, Pose& out) {
    t += dtMs / 1000.0f;
    if (t >= dur) { out = makePose(0, restA); stop(); return false; }
    Pose p = makePose();
    uint8_t f = 0;
    mv->fn(t, t * mv->bpm * rate / 60.0f, p, f);
    if (f) wantFace = f;
    if (wantFace && wantFace != shownFace && (!titled || t > 1.3f)) { shownFace = wantFace; if (onFace) onFace(wantFace); }
    float bi = ease(t / blendIn), bo = ease((dur - t) / 0.35f);
    for (int i = 0; i < LEG_N; i++) {
      float s = from.s[i] + (p.s[i] - from.s[i]) * bi;
      float a = from.a[i] + (p.a[i] - from.a[i]) * bi;
      out.s[i] = s * bo;
      out.a[i] = restA + (a - restA) * bo;
    }
    return true;
  }

  bool tickKF(uint32_t dtMs, Pose& out) {
    elapsed += dtMs;
    while (elapsed >= kf[idx].ms) {
      elapsed -= kf[idx].ms;
      prev = from;
      prevMs = kf[idx].ms;
      for (int i = 0; i < LEG_N; i++) { from.s[i] = kf[idx].s[i]; from.a[i] = kf[idx].a[i]; }
      idx++;
      if (idx >= kf.size()) { out = from; active = false; return false; }
      fireFace();
    }
    const KF& k = kf[idx];
    const KF* n = idx + 1 < kf.size() ? &kf[idx + 1] : nullptr;
    float T = k.ms, u = (float)elapsed / T, tn = n ? n->ms : 0;
    for (int i = 0; i < LEG_N; i++) {
      out.s[i] = herm(prev.s[i], from.s[i], k.s[i], n ? n->s[i] : k.s[i], prevMs, T, tn, u);
      out.a[i] = herm(prev.a[i], from.a[i], k.a[i], n ? n->a[i] : k.a[i], prevMs, T, tn, u);
    }
    return true;
  }
};

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
