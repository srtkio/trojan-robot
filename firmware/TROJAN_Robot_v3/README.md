# TROJAN firmware 3.0

Written from scratch for the 8-servo quadruped (ESP32 DevKit, SSD1306 OLED). Open `TROJAN_Robot_v3.ino` in Arduino IDE
(board: ESP32 Dev Module, libraries: ESP32Servo, Adafruit GFX, Adafruit SSD1306). Everything else is in this folder.

## New in 3.0
**Remote, redesigned** (plain words, no emojis, checked on 12 screen sizes from small phones to laptops)
- Four tabs: **Drive** (joystick, Stand up / Sit down / Hold to run, speed, quick moves), **Play** (tricks, moving
  tricks, dances, your dances), **Face** (moods, animations, message), **More** (everything advanced, folded away).
- The live leg view is back on Drive and moves smoothly; a Legs / Face switch shows TROJAN's screen instead.
- One joystick (up walks, sideways turns) or two (left moves, right rotates, right up/down sets body height).
  Sideways phones and laptops start with two; the choice in More > Driving controls is remembered.
- More keeps every 2.0 tool: walking style, drift fix (steering correction + per-leg tuning), route recorder,
  Pose lab + dance builder (every leg by hand), Face studio, servo test (every motor by hand), behaviour, Wi-Fi, system.
- More > All motors at once: all 8 motor angles on sliders together (as in the original Sesame remote).
- Play > "How big tricks and dances move" (40-100 %, default 100 %, not saved). Pose lab > "Copy TROJAN's pose".
  My dances: "Save file" downloads a dance, "Open a dance file" adds one.
- A "Can't reach TROJAN" bar explains what to do when the connection drops.

**Wi-Fi**: one device at a time (the radio refuses a second one). The password can be changed in More > Wi-Fi;
TROJAN restarts and shows the current password on its screen at start-up. Saved tuning is not affected.
- The limit is per device: a phone that joined before rejoins by itself when nearby and takes the only slot. To hand
  over, the current driver turns Wi-Fi off or forgets Sesame-Controller. A phone that walks out of range is dropped
  after 30 s of silence.
- The default password is printed on TROJAN's screen, so change it. After a change, every phone must forget the
  network and join again with the new password.

**Feels alive**
- Eyes have pupils with a glint that follow the gaze (small when startled or scared, wide when excited), breathe
  slowly, and blink on big glances.
- Every mood moves: angry shakes and shows an anger mark, surprised pops open with a "!", happy bobs, wink twinkles,
  suspicious side-eyes, bored droops with "...", determined streams the ground past (auto while running).
- Idle personality: boredom grows over two minutes. Most idle acts are face-only (servos stay cool); body gestures get
  likelier as it gets bored, the eyes announce each one 0.7 s before the body moves, and nothing repeats twice in a row.

**Fixed**
- Remote: an error reply no longer shows TROJAN as offline; every request times out, so one hung drive request can
  no longer freeze driving; the dance preview is no longer cancelled by the pose keep-alive; the live view no longer
  stops on older iPhones.
- The "Bound" gait button is gone (it was never implemented and silently ran Run). Gait modes are 0-2.
- Showing a Face Studio slot reads the file before taking the face lock, so the OLED task never waits on flash.

## New in 2.0
**Motion**
- Every built-in move is a continuous wave evaluated 50 times a second: no keyframe stops, no jerks.
- Moves use the full safe range: hips +-45, a raised paw up to 120. Micro servos look smooth on big brisk moves, twitchy on tiny slow ones, so the idle breathing from the first v2 draft is gone.
- Moves and running get the servos' full speed (800 deg/s slew); normal walking keeps the tuned limit.
- Any move that lifts ONE leg braces first: diagonal corner sinks, same-side corner a bit, neighbours step toward the lifted leg.
- Pose Lab dances flow through keyframes (monotone cubic) instead of stopping at each one.

**Gaits** (Drive tab)
| mode | legs in the air | feel |
|---|---|---|
| Crawl | one at a time (FR, RL, FL, RR), 3 always down | slow, rock steady (same as 1.0) |
| Trot | diagonal pairs: FR+RL, then RR+FL | quicker, still balanced (same as 1.0) |
| Run | trot timing, 1.7x cadence, longer stride | the dog run |
Hold **RUN** (or Shift) to sprint from Crawl/Trot. Switching mode mid-walk blends the leg timing over 0.3 s.

**Tricks**: Hi (braced, paw high, big wave), Give paw, Yes, No, Play bow + tail wag, Sit pretty, Stretch, Push-ups, Wiggle, Hula, Tap, Pee, Scratch, Sneeze, Shiver, Pounce, Play dead (belly flop + heart monitor), Look around.
**Drive tricks**: Spin, Circle, Zigzag, Figure 8 (they walk, give them room; the sticks cancel them).
**Dances** (beat-locked, tempo slider 0.6x-1.6x): Groove, Shuffle, Disco, Twerk, Headbang, Flow, Ripple, Robot. Each opens with a title card.

**Face**: shape-morphing eyes at 25 fps on the second core, moods Curious, Scared, Bored, Laugh, Determined (auto while running), Cool wears shades, eyes look where TROJAN walks, dance title card, heart monitor screen.

**Remote**: quick-move chips (keys 1-0), Chill/Normal/Zoom presets, RUN button, 3-mode gait picker with explanations, trick and drive-trick grids from the firmware, Surprise me, dance tempo, path recorder, live OLED mirror, haptics.

**Serial**: `list` (all moves), `mode 0-2`.

## Use
1. Power TROJAN. It stands up, looks around, then the OLED shows the Wi-Fi details.
2. Join Wi-Fi **Sesame-Controller** with the password shown on TROJAN's screen (default **12345678**). Only one device
   can be connected at a time. The remote opens by itself (captive portal), or go to `192.168.4.1` / `trojan.local`.
3. The OLED shows CONNECTED and TROJAN waves.

## Files
| file | what |
|---|---|
| `config.h` | pins, safe servo limits, leg/servo naming (measured on the robot) |
| `robot.h` | servo bus (slew limit, sub-trim, release) + leg kinematics |
| `gait.h` | settings (saved in flash) + walking engine (crawl / trot, strafe, turn on the spot) |
| `anim.h` | keyframe player, gestures, 4 built-in dances, user dances |
| `face.h` | OLED engine: 17 moods (pupils, breathing, per-mood animation), animated screens, Face Studio bitmaps, text banner |
| `ctrl.h` | the 50 Hz brain: priorities, dead-man, sleep, idle behaviour |
| `web.h`, `remote.h` | HTTP API and the remote page |

## How the legs work (measured)
Each leg = hip servo (swings the leg round the body corner) + foot servo (splays the lower leg sideways: 0 tall, 90 flat).
Leg space: `s` = hip swing in degrees (+ forward), `a` = foot splay in degrees.
Raw angles: `R1=135+s R2=45+s L1=45-s L2=135-s`; feet `R4=a L3=a R3=160-a L4=160-a`. Raw limits are always enforced.

## Tuning the walk (rubber feet)
Remote > More > Walking style: speed, step length, foot lift, body height.
Remote > More > Fix drifting: **steering correction** (TROJAN curves right? move toward "Steer left") and per-leg
step length / hip offset / lift. Remote > More > Servo test: motor offsets. Press Save to keep them across reboots.

## Safety
- Dead-man: if the remote stops talking for 500 ms (adjustable) the stick input is ignored.
- E-STOP (button or Esc): cancels everything and stands.
- Every angle is clamped to the tested range, and servo speed is limited (default 420 deg/s).
- Servos are released a few seconds after sitting (no buzzing) and wake on any command.

## Serial console (115200)
`c x y w` drive, `list`, `mode 0-2`, `stand`, `sit`, `relax`, `estop`, `play <name>`, `raw <servo> <angle>`, `s` (state JSON).
