# TROJAN firmware 1.0

Written from scratch for the 8-servo quadruped (ESP32 DevKit, SSD1306 OLED). Open `TROJAN_Robot.ino` in Arduino IDE
(board: ESP32 Dev Module, libraries: ESP32Servo, Adafruit GFX, Adafruit SSD1306). Everything else is in this folder.

## Use
1. Power TROJAN. It stands up, looks around, then the OLED shows the Wi-Fi details.
2. Join Wi-Fi **Sesame-Controller** / **12345678**. The remote opens by itself (captive portal), or go to `192.168.4.1` / `trojan.local`.
3. The OLED shows CONNECTED and TROJAN waves.

## Files
| file | what |
|---|---|
| `config.h` | pins, safe servo limits, leg/servo naming (measured on the robot) |
| `robot.h` | servo bus (slew limit, sub-trim, release) + leg kinematics |
| `gait.h` | settings (saved in flash) + walking engine (crawl / trot, strafe, turn on the spot) |
| `anim.h` | keyframe player, gestures, 4 built-in dances, user dances |
| `face.h` | OLED engine: 12 moods, 7 animated screens, Face Studio bitmaps, text banner |
| `ctrl.h` | the 50 Hz brain: priorities, dead-man, sleep, idle behaviour |
| `web.h`, `remote.h` | HTTP API and the remote page |

## How the legs work (measured)
Each leg = hip servo (swings the leg round the body corner) + foot servo (splays the lower leg sideways: 0 tall, 90 flat).
Leg space: `s` = hip swing in degrees (+ forward), `a` = foot splay in degrees.
Raw angles: `R1=135+s R2=45+s L1=45-s L2=135-s`; feet `R4=a L3=a R3=160-a L4=160-a`. Raw limits are always enforced.

## Tuning the walk (rubber feet)
Remote > Drive: speed, stride, lift, stance height, **steer trim** (cancels a constant drift).
Remote > Tune: per-leg stride / hip offset / lift, servo sub-trims. Press Save to keep them across reboots.

## Safety
- Dead-man: if the remote stops talking for 500 ms (adjustable) the stick input is ignored.
- E-STOP (button or Esc): cancels everything and stands.
- Every angle is clamped to the tested range, and servo speed is limited (default 420 deg/s).
- Servos are released a few seconds after sitting (no buzzing) and wake on any command.

## Serial console (115200)
`c x y w` drive, `stand`, `sit`, `relax`, `estop`, `play <name>`, `raw <servo> <angle>`, `s` (state JSON).
