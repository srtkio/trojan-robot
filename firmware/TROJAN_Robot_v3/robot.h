#pragma once
// Servo bus + leg kinematics. Leg space: per leg a hip swing s (deg, + = forward) and a
// foot splay a (deg, 0 = leg straight down). Everything above this file thinks in leg space.
#include <ESP32Servo.h>
#include "config.h"

struct Pose {
  float s[LEG_N];
  float a[LEG_N];
};

inline Pose makePose(float s = 0, float a = 0) {
  Pose p;
  for (int i = 0; i < LEG_N; i++) { p.s[i] = s; p.a[i] = a; }
  return p;
}

// Measured on the robot: every hip turns counter-clockwise (seen from above) as its raw angle rises,
// and the right-hand hips are mounted rotated 180 deg relative to the left-hand ones.
inline float hipRaw(uint8_t leg, float s) {
  switch (leg) {
    case FR: return 135 + s;
    case RR: return 45 + s;
    case FL: return 45 - s;
    default: return 135 - s;  // RL
  }
}
// R4 and L3 splay with rising angle, R3 and L4 splay with falling angle (160 = down).
inline float footRaw(uint8_t leg, float a) { return (leg == RR || leg == FL) ? a : 160 - a; }

struct ServoBus {
  Servo dev[SERVO_N];
  float cur[SERVO_N];
  float tgt[SERVO_N];
  int16_t lastUs[SERVO_N];
  int8_t trim[SERVO_N];
  float maxSpeed = 420;  // deg/s slew limit, the last line of defence against snapping moves
  bool attached = false;

  float clampRaw(uint8_t i, float raw) const {
    return constrain(raw + trim[i], (float)SERVO_MIN[i], (float)SERVO_MAX[i]);
  }

  void setPose(const Pose& p) {
    for (uint8_t l = 0; l < LEG_N; l++) {
      tgt[LEG_HIP[l]]  = clampRaw(LEG_HIP[l], hipRaw(l, constrain(p.s[l], -MAX_SWING, MAX_SWING)));
      tgt[LEG_FOOT[l]] = clampRaw(LEG_FOOT[l], footRaw(l, constrain(p.a[l], 0.0f, MAX_SPLAY)));
    }
  }

  void setRaw(uint8_t i, float raw) { if (i < SERVO_N) tgt[i] = clampRaw(i, raw); }

  void begin(const Pose& p) {
    for (int i = 0; i < SERVO_N; i++) { trim[i] = 0; lastUs[i] = -1; }
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    setPose(p);
    for (int i = 0; i < SERVO_N; i++) cur[i] = tgt[i];
    attachAll();
  }

  void attachAll() {
    if (attached) return;
    for (int i = 0; i < SERVO_N; i++) {
      dev[i].setPeriodHertz(50);
      dev[i].attach(SERVO_PIN[i], PULSE_MIN_US, PULSE_MAX_US);
      lastUs[i] = -1;
    }
    attached = true;
    write(true);
  }

  void detachAll() {
    if (!attached) return;
    for (int i = 0; i < SERVO_N; i++) dev[i].detach();
    attached = false;
  }

  void write(bool force) {
    for (int i = 0; i < SERVO_N; i++) {
      int us = PULSE_MIN_US + (int)(cur[i] * (PULSE_MAX_US - PULSE_MIN_US) / 180.0f + 0.5f);
      if (force || us != lastUs[i]) { dev[i].writeMicroseconds(us); lastUs[i] = us; }
    }
  }

  void tick(float dt) {
    float step = maxSpeed * dt;
    for (int i = 0; i < SERVO_N; i++) {
      float d = tgt[i] - cur[i];
      cur[i] += constrain(d, -step, step);
    }
    if (attached) write(false);
  }

  bool settled() const {
    for (int i = 0; i < SERVO_N; i++) if (fabsf(tgt[i] - cur[i]) > 0.3f) return false;
    return true;
  }
};
