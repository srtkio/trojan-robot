#pragma once
// TROJAN - hardware constants. Everything here was measured on the real robot.
#include <Arduino.h>

#define ROBOT_NAME "TROJAN"
#define FW_VERSION "3.0"
#define AP_SSID    "Sesame-Controller"   // kept from the old firmware so phones reconnect
#define AP_PASS    "12345678"            // default only; the owner can change it from the remote
#define AP_MAX_CLIENTS 1                 // one driver at a time: the radio refuses a second device
#define HOSTNAME   "trojan"

#define OLED_W    128
#define OLED_H    64
#define OLED_ADDR 0x3C
#define PIN_SDA   21
#define PIN_SCL   22

// Hardware servo index order (matches the wiring): R1 R2 L1 L2 R4 R3 L3 L4
enum SId : uint8_t { S_R1, S_R2, S_L1, S_L2, S_R4, S_R3, S_L3, S_L4, SERVO_N };
static const uint8_t SERVO_PIN[SERVO_N] = {14, 27, 13, 12, 33, 32, 25, 26};
// Tested safe raw angle range per servo. Never exceeded, whatever the caller asks for.
static const int16_t SERVO_MIN[SERVO_N] = {80, 0, 0, 80, 0, 0, 0, 0};
static const int16_t SERVO_MAX[SERVO_N] = {180, 100, 100, 180, 160, 160, 160, 160};
#define PULSE_MIN_US 500
#define PULSE_MAX_US 2400

// Legs. Each leg = one hip servo (swings the leg around the body corner)
// + one foot servo (splays the lower leg sideways: 0 = down/tall, 90 = flat on the table).
enum Leg : uint8_t { FR, RR, FL, RL, LEG_N };
static const uint8_t LEG_HIP[LEG_N]  = {S_R1, S_R2, S_L1, S_L2};
static const uint8_t LEG_FOOT[LEG_N] = {S_R3, S_R4, S_L3, S_L4};
static const char*   LEG_NAME[LEG_N] = {"FR", "RR", "FL", "RL"};

// Full usable range. Hips: the tested raw limits give every hip exactly +-45 in leg space (front hips
// could go 10 deg further back and rear hips 10 further forward, but then same-side legs can cross).
// Feet: raw 0..160 = splay 0..160; past ~95 the leg points upward, so 120 is a raised paw. Standing
// and walking never come near that; only single lifted legs and Play dead use it.
#define MAX_SWING   45.0f
#define MAX_SPLAY  120.0f
