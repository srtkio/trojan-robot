# TROJAN firmware 2.0

Written from scratch for the 8-servo quadruped (ESP32 DevKit, SSD1306 OLED). Open `TROJAN_Robot_v2.ino` in Arduino IDE
(board: ESP32 Dev Module, libraries: ESP32Servo, Adafruit GFX, Adafruit SSD1306). Everything else is in this folder.

## New in 2.0
**Motion**
- Every built-in move is a continuous wave evaluated 50 times a second: no keyframe stops, no jerks.
- Moves use the full safe range: hips +-45, a raised paw up to 120. Micro servos look smooth on big brisk moves, twitchy on tiny slow ones, so the idle breathing from the first v2 draft is gone.
- Moves and running get the servos' full speed (1000 deg/s slew); normal walking keeps the tuned limit.
- Any move that lifts ONE leg braces first: diagonal corner sinks, same-side corner a bit, neighbours step toward the lifted leg.
- Pose Lab dances flow through keyframes (monotone cubic) instead of stopping at each one.

**Gaits** (Drive tab)
| mode | legs in the air | feel |
|---|---|---|
| Crawl | one at a time (FR, RL, FL, RR), 3 always down | slow, rock steady (same as 1.0) |
| Trot | diagonal pairs: FR+RL, then RR+FL | quicker, still balanced (same as 1.0) |
| Run | trot timing, 1.7x cadence, longer stride | the dog run |
| Bound | front pair, then rear pair | gallop / bunny hop |
Hold **RUN** (or Shift) to sprint from Crawl/Trot. Switching mode mid-walk blends the leg timing over 0.3 s.

**Tricks**: Hi (braced, paw high, big wave), Give paw, Yes, No, Play bow + tail wag, Sit pretty, Stretch, Push-ups, Wiggle, Hula, Tap, Pee, Scratch, Sneeze, Shiver, Pounce, Play dead (belly flop + heart monitor), Look around.
**Drive tricks**: Spin, Circle, Zigzag, Figure 8 (they walk, give them room; the sticks cancel them).
**Dances** (beat-locked, tempo slider 0.6x-1.6x): Groove, Shuffle, Disco, Twerk, Headbang, Flow, Ripple, Robot. Each opens with a title card.

**Face**: shape-morphing eyes at 25 fps on the second core, moods Curious, Scared, Bored, Laugh, Determined (auto while running), Cool wears shades, eyes look where TROJAN walks, dance title card, heart monitor screen.

**Remote**: quick-move chips (keys 1-0), Chill/Normal/Zoom presets, RUN button, 4-mode gait picker with explanations, trick and drive-trick grids from the firmware, Surprise me, dance tempo, path recorder, live OLED mirror, haptics.

**Serial**: `list` (all moves), `mode 0-3`.

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
`c x y w` drive, `list`, `mode 0-3`, `stand`, `sit`, `relax`, `estop`, `play <name>`, `raw <servo> <angle>`, `s` (state JSON).
