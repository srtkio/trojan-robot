#pragma once
// Show layer: smooth keyframe motion (glide), procedural OLED screens, startup, dances.
// Included by SesameRobot_Custom_v2.ino after the globals/prototypes it uses.

// ====== PROCEDURAL SCREENS (drawn with Adafruit GFX, no bitmap flash cost) ======
enum ProcMode : uint8_t {
  PROC_NONE, PROC_BOOT, PROC_WIFI, PROC_CONNECTED,
  PROC_EQ, PROC_HEARTS, PROC_EYES, PROC_STARS, PROC_DIZZY, PROC_GLITCH
};
ProcMode procMode = PROC_NONE;
unsigned long procStartMs = 0;
unsigned long procLastMs = 0;
bool inShow = false;       // a dance/startup is driving the screen itself
String showName = "";      // glide()/hold() abort when currentCommand stops matching this
int servoCmd[8] = {90, 90, 90, 90, 90, 90, 90, 90};  // last requested angle per servo (pre-subtrim)

void procStart(ProcMode m) { procMode = m; procStartMs = millis(); procLastMs = 0; }

void procEnd() {
  procMode = PROC_NONE;
  if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
    updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
  }
}

static void drawHeart(int x, int y, int r) {
  display.fillCircle(x - r, y, r, SSD1306_WHITE);
  display.fillCircle(x + r, y, r, SSD1306_WHITE);
  display.fillTriangle(x - 2 * r, y + 1, x + 2 * r, y + 1, x, y + 2 * r + 2, SSD1306_WHITE);
}

static void drawSparkle(int x, int y, int s) {
  display.drawFastHLine(x - s, y, 2 * s + 1, SSD1306_WHITE);
  display.drawFastVLine(x, y - s, 2 * s + 1, SSD1306_WHITE);
  if (s > 2) {
    display.drawLine(x - s / 2, y - s / 2, x + s / 2, y + s / 2, SSD1306_WHITE);
    display.drawLine(x - s / 2, y + s / 2, x + s / 2, y - s / 2, SSD1306_WHITE);
  }
}

static void drawEyePair(int lx, int ly, int h) {
  for (int e = 0; e < 2; e++) {
    int cx = e ? 94 : 34, cy = 32;
    display.fillRoundRect(cx - 15, cy - h / 2, 30, h, 9, SSD1306_WHITE);
    if (h > 14) display.fillCircle(cx + lx, cy + ly, 5, SSD1306_BLACK);
  }
}

static void drawProc(unsigned long t) {
  switch (procMode) {
    case PROC_BOOT: {
      int letters = min(6UL, t / 180);
      display.setTextSize(3);
      display.setCursor(10, 12);
      for (int i = 0; i < letters; i++) display.print("SESAME"[i]);
      display.drawFastHLine(10, 40, min(108UL, t / 6), SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(34, 50);
      display.print("waking up");
      for (unsigned long d = 0; d < (t / 300) % 4; d++) display.print('.');
      break;
    }
    case PROC_WIFI: {
      display.fillCircle(18, 24, 2, SSD1306_WHITE);
      int arcs = 1 + (t / 400) % 3;
      for (int r = 1; r <= arcs; r++)
        for (int a = -45; a <= 45; a += 3)
          display.drawPixel(18 + r * 6 * sin(a * DEG_TO_RAD), 24 - r * 6 * cos(a * DEG_TO_RAD), SSD1306_WHITE);
      display.setTextSize(2);
      display.setCursor(42, 4);
      display.print("WiFi");
      display.setTextSize(1);
      display.setCursor(0, 32);
      display.print(AP_SSID);
      display.setCursor(0, 42);
      display.print("Pass: " AP_PASS);
      display.setCursor(0, 54);
      display.print(WiFi.softAPIP().toString());
      break;
    }
    case PROC_CONNECTED: {
      float p = min(1.0f, t / 500.0f);
      int bx = 40 + (int)(14 * min(1.0f, p / 0.4f)), by = 22 + (int)(14 * min(1.0f, p / 0.4f));
      for (int o = 0; o < 3; o++) display.drawLine(40, 22 + o, bx, by + o, SSD1306_WHITE);
      if (p > 0.4f) {
        float q = (p - 0.4f) / 0.6f;
        for (int o = 0; o < 3; o++) display.drawLine(54, 36 + o, 54 + (int)(34 * q), 36 - (int)(30 * q) + o, SSD1306_WHITE);
      }
      display.setTextSize(2);
      display.setCursor(10, 46);
      display.print("CONNECTED");
      break;
    }
    case PROC_EQ:
      for (int i = 0; i < 16; i++) {
        int h = 4 + (int)((sin(t / 110.0 + i * 0.7) + sin(t / 170.0 - i * 1.3) + 2) * 12);
        display.fillRect(i * 8, 63 - h, 6, h, SSD1306_WHITE);
      }
      break;
    case PROC_HEARTS:
      for (int i = 0; i < 6; i++) {
        int x = (i % 2 ? 100 : 8) + (i / 2) * 9 + (int)(sin((t + i * 300) / 500.0) * 4);
        int y = 70 - (int)((t / 18 + i * 37) % 84);
        drawHeart(x, y, 3 + i % 2);
      }
      drawHeart(64, 22, 9 + (int)(3 * sin(t / 150.0)));
      break;
    case PROC_EYES: {
      int h = (t % 2800 > 2650) ? 6 : 34;  // blink
      drawEyePair((int)(sin(t / 600.0) * 8), (int)(cos(t / 900.0) * 4), h);
      break;
    }
    case PROC_STARS:
      for (int i = 0; i < 8; i++)
        drawSparkle(10 + (i * 37) % 112, 8 + (i * 23) % 48, 1 + (int)(3 * fabs(sin((t + i * 250) / 300.0))));
      break;
    case PROC_DIZZY:
      for (int e = 0; e < 2; e++) {
        int cx = e ? 94 : 34;
        for (int k = 0; k < 40; k++) {
          float a = k * 0.5 + t / 100.0, r = k * 0.36;
          display.drawPixel(cx + r * cos(a), 32 + r * sin(a), SSD1306_WHITE);
        }
      }
      break;
    case PROC_GLITCH:
      display.setTextSize(3);
      display.setCursor(19 + random(-2, 3), 20 + random(-2, 3));
      display.print("ROBOT");
      for (int i = 0; i < 3; i++)
        display.fillRect(random(0, 128), random(0, 64), random(10, 50), 2, SSD1306_INVERSE);
      break;
    default: break;
  }
}

void procTick() {
  unsigned long now = millis();
  if (now - procLastMs < 80) return;  // an OLED refresh costs ~25 ms over I2C
  procLastMs = now;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  drawProc(now - procStartMs);
  display.display();
  display.setTextWrap(true);
}

// ====== SMOOTH MOTION ======
// Moves every servo from where it last was to target[] over ms with ease-in-out.
// target[i] < 0 leaves that servo alone. Returns false if currentCommand changed from `keep`.
bool glide(const int* target, unsigned ms, String keep) {
  int from[8];
  for (int i = 0; i < 8; i++) from[i] = servoCmd[i];
  unsigned long t0 = millis();
  for (;;) {
    unsigned long e = millis() - t0;
    float k = e >= ms ? 1.0f : (float)e / ms;
    k = k * k * (3 - 2 * k);
    for (int i = 0; i < 8; i++)
      if (target[i] >= 0) setServoAngle(i, (int)(from[i] + (target[i] - from[i]) * k + 0.5f));
    if (e >= ms) return true;
    delayWithFace(12);
    if (currentCommand != keep) return false;
  }
}

bool hold(unsigned ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    delayWithFace(12);
    if (currentCommand != showName) return false;
  }
  return true;
}

// Dance vocabulary. tw = all hips twist the same way, sp = right hips out / left hips in,
// lift = feet up bitmask (1 R4, 2 R3, 4 L3, 8 L4). Keep |tw|+|sp| <= 45 to stay inside the safe limits.
bool mv(int tw, int sp, uint8_t lift, unsigned ms) {
  int t[8] = {135 + tw + sp, 45 + tw + sp, 45 + tw - sp, 135 + tw - sp,
              (lift & 1) ? 45 : 0, (lift & 2) ? 135 : 160, (lift & 4) ? 45 : 0, (lift & 8) ? 135 : 160};
  return glide(t, ms, showName);
}
#define M(...) if (!mv(__VA_ARGS__)) return false;
#define H(ms)  if (!hold(ms)) return false;

// ====== DANCES (~25 s each) ======
bool danceGroove() {
  procStart(PROC_EQ);
  for (int i = 0; i < 8; i++) { M(0, 0, 15, 250); M(0, 0, 0, 250); }          // bounce, 4 s
  for (int i = 0; i < 8; i++) { M(25, 0, 0, 500); M(-25, 0, 0, 500); }        // sway, 8 s
  procStart(PROC_HEARTS);
  for (int i = 0; i < 8; i++) { M(15, 0, 5, 500); M(-15, 0, 10, 500); }       // side stomps, 8 s
  procStart(PROC_EQ);
  for (int i = 0; i < 4; i++) { M(0, 25, 0, 500); M(0, -25, 0, 500); }        // spread pulses, 4 s
  procStart(PROC_STARS);
  M(0, 0, 15, 600); M(0, 0, 0, 600);                                          // finish
  return true;
}

bool danceShuffle() {
  procStart(PROC_EYES);
  for (int i = 0; i < 8; i++) { M(20, 0, 5, 400); M(-20, 0, 10, 400); }       // diagonal shuffle, 6.4 s
  for (int i = 0; i < 8; i++) { M(0, 25, 0, 400); M(0, -25, 15, 400); }       // 6.4 s
  procStart(PROC_DIZZY);
  for (int i = 0; i < 4; i++) { M(40, 0, 5, 800); M(-40, 0, 10, 800); }       // big twists, 6.4 s
  procStart(PROC_STARS);
  for (int i = 0; i < 16; i++) { M(15, 0, 0, 200); M(-15, 0, 15, 200); }      // quick wiggle, 6.4 s
  M(0, 0, 0, 500);
  return true;
}

bool danceRobot() {
  struct Pose { int8_t tw, sp; uint8_t lift; };
  static const Pose seq[8] = {{20, 0, 0}, {-20, 0, 0}, {0, 20, 0}, {0, -20, 0},
                              {0, 0, 15}, {0, 0, 5}, {0, 0, 10}, {20, 20, 15}};
  procStart(PROC_GLITCH);
  for (int r = 0; r < 5; r++)
    for (int i = 0; i < 8; i++) { M(seq[i].tw, seq[i].sp, seq[i].lift, 150); H(450); }  // snap + hold, 24 s
  procStart(PROC_EQ);
  M(0, 0, 15, 500);
  return true;
}

void runShow(const char* name, bool (*body)()) {
  Serial.println(name);
  showName = currentCommand;
  inShow = true;
  bool done = body();
  inShow = false;
  procEnd();
  runStandPose(1);
  if (done && currentCommand == name) currentCommand = "";
}

// ====== STARTUP / WIFI SCREENS ======
// atBoot: finish on the WiFi screen and wait for a client; otherwise just go back to the idle face.
void runStartup(bool atBoot) {
  static const int rest[8] = {135, 45, 45, 135, 90, 90, 90, 90};
  static const int stand[8] = {135, 45, 45, 135, 0, 160, 0, 160};
  showName = currentCommand;
  inShow = true;
  procStart(PROC_BOOT);
  for (int i = 0; i < 8; i++) setServoAngle(i, rest[i]);  // first command after power-up: legs jump here once
  bool ok = hold(1300)                                    // logo plays while the legs settle
         && glide(stand, 900, showName)                   // stand up
         && mv(20, 0, 0, 300) && mv(-20, 0, 0, 300) && mv(0, 0, 0, 300)  // look around
         && mv(0, 0, 15, 350) && mv(0, 0, 0, 350);        // stretch
  inShow = false;
  if (atBoot) {
    procStart(PROC_WIFI);
  } else {
    procEnd();
    enterIdle();
  }
  (void)ok;
}

// Called from loop(): waits for the first WiFi client, shows "Connected!", and clears stale screens.
void showTick() {
  static unsigned long lastPoll = 0;
  if (inShow || procMode == PROC_NONE) return;
  if (currentCommand != "") { procEnd(); return; }  // a command takes over from any idle/manual screen
  if (procMode == PROC_WIFI && millis() - lastPoll > 300) {
    lastPoll = millis();
    if (WiFi.softAPgetStationNum() > 0) procStart(PROC_CONNECTED);
  } else if (procMode == PROC_CONNECTED && millis() - procStartMs > 2500) {
    procEnd();
    enterIdle();
  }
}
