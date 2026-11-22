# Sesame Robot - customized firmware for your ESP32 DevKit

## What this version changes

- Keeps the original Sesame firmware's Wi-Fi web remote, OLED animation system, movement library, captive portal, and serial CLI.
- Uses your tested servo GPIO wiring in the original movement library's index order.
- Uses your tested per-motor angle limits in the same index order.
- Uses OLED I2C on GPIO 21 (SDA) and GPIO 22 (SCL), which are free in your servo map.
- Uses the 500-2400 us pulse range used in your own MG90S test sketches. Do not change this without recalibrating the safe angle limits.
- Enforces each motor's limits in the single `setServoAngle()` path. That covers movement sequences, serial commands, and web manual motor control.

## Required project files

This `.ino` file is not standalone. Keep these three original project headers in the SAME Arduino sketch folder:

- `face-bitmaps.h`
- `movement-sequences.h`
- `captive-portal.h`

Get them from the official repository's `firmware` directory:

https://github.com/dorianborian/sesame-robot/tree/main/firmware

Do not rename the headers. The project folder should look like:

```text
SesameRobot_Custom/
  SesameRobot_Custom.ino
  face-bitmaps.h
  movement-sequences.h
  captive-portal.h
```

## Arduino IDE libraries

Install through Library Manager:

- ESP32Servo (the original firmware README recommends v3.0.9)
- Adafruit GFX Library
- Adafruit SSD1306

The following are included with the ESP32 Arduino core: WiFi, WebServer, DNSServer, ESPmDNS, Wire.

Select the board matching your module (for a classic ESP32 DevKit, usually `ESP32 Dev Module`).

## Your pin mapping

The original movement library's enum/index order is `R1, R2, L1, L2, R4, R3, L3, L4`.

| Library index | Motor | ESP32 GPIO | Safe min | Safe max |
|---:|---|---:|---:|---:|
| 0 | R1 | 14 | 80 | 180 |
| 1 | R2 | 27 | 0 | 100 |
| 2 | L1 | 13 | 0 | 100 |
| 3 | L2 | 12 | 80 | 180 |
| 4 | R4 | 33 | 0 | 160 |
| 5 | R3 | 32 | 0 | 160 |
| 6 | L3 | 25 | 0 | 160 |
| 7 | L4 | 26 | 0 | 160 |

OLED:

- VCC -> 3V3
- GND -> GND
- SDA -> GPIO 21
- SCL -> GPIO 22
- I2C address -> 0x3C (as configured in the sketch)

## Remote control

1. Flash the firmware and open Serial Monitor at 115200 baud for startup diagnostics.
2. Power the ESP32 and servo rail. The ESP32 creates a Wi-Fi access point:
   - SSID: `Sesame-Controller`
   - Password: `12345678`
3. Connect your phone/laptop to that Wi-Fi network.
4. Open `http://192.168.4.1` in a browser. The captive portal may also open automatically.
5. Use the directional pad for forward/back/left/right (hold a direction; release to stop), the pose buttons for animations, Settings for speed/cycle settings and Wi-Fi provisioning, and Settings -> Manual Motor Control for individual servo sliders.
6. The web sliders use the matching per-motor min/max after applying the optional patch below. Even without the patch, the firmware clamps every motor command in `setServoAngle()` so the motors cannot be commanded outside the defined safe ranges through this firmware path.

## Optional: make the web sliders visibly match safe limits

In `captive-portal.h`, find the eight `<input type="range" id="motor...">` controls and change only their `min` and `max` attributes to:

| Slider | Label | min | max |
|---:|---|---:|---:|
| motor1 | R1 | 80 | 180 |
| motor2 | R2 | 0 | 100 |
| motor3 | L1 | 0 | 100 |
| motor4 | L2 | 80 | 180 |
| motor5 | R4 | 0 | 160 |
| motor6 | R3 | 0 | 160 |
| motor7 | L3 | 0 | 160 |
| motor8 | L4 | 0 | 160 |

The matching controls are around the `Manual Motor Control` section. This only changes the visible slider endpoints; the firmware-level clamp is the actual safety guard.

## Important mechanical/power notes

- These limits are the safe ranges you tested, in degrees, assuming the same `500-2400 us` pulse calibration used in your test code.
- Several original Sesame poses request angles outside your custom ranges. The firmware will clamp those requests, so some stock poses may look different from the original robot. This is intentional for safety.
- Use a regulated external 5V supply sized for the eight MG90S servos. Do not power the eight servos from the ESP32 3V3 pin or its onboard regulator.
- Join the servo supply GND and ESP32 GND. Keep OLED power on 3V3 unless your display module explicitly supports another voltage.
- First test with the robot lifted and verify that the correct named motor responds to each remote slider.
