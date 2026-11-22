#pragma once
// OLED face engine v2. Eyes are shapes (size, roundness, upper-lid slant, lower-lid curve) that MORPH
// between moods, blink with squash, and shift with gaze in perspective. Rendering runs at ~25 fps in its
// own task on core 0, so the I2C transfer never stalls the 50 Hz servo loop on core 1.
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "config.h"

// Face ids: 1..17 moods, 32..40 animated screens, 48..50 system screens, 64..71 custom bitmaps.
enum : uint8_t {
  FC_NEUTRAL = 1, FC_HAPPY, FC_SAD, FC_ANGRY, FC_SURPRISED, FC_SLEEPY, FC_LOVE, FC_EXCITED, FC_WINK, FC_DIZZY, FC_SUSPICIOUS, FC_COOL,
  FC_CURIOUS, FC_SCARED, FC_BORED, FC_LAUGH, FC_DETERMINED, FC_MOOD_LAST = FC_DETERMINED,
  FC_EQ = 32, FC_HEARTS, FC_STARS, FC_SPIRAL, FC_GLITCH, FC_NOTES, FC_MATRIX, FC_TITLE, FC_ECG,
  FC_BOOT = 48, FC_WIFI, FC_CONNECTED,
  FC_BMP0 = 64
};

static inline uint32_t umin(uint32_t a, uint32_t b) { return a < b ? a : b; }

// One eye: w,h,r size and corner radius; x,y offset from its home; lidIn/lidOut = px of upper lid at the
// nose side / outer side (slant = emotion); lidLow = px pushed up by a curved lower lid (smile crescent).
struct EyeShape { float w, h, r, x, y, lidIn, lidOut, lidLow; };

static const EyeShape EYES[FC_MOOD_LAST + 1][2] = {   // [mood][left, right]
  {{36, 40, 10, 0, 0, 0, 0, 0}, {36, 40, 10, 0, 0, 0, 0, 0}},          // 0 unused
  {{36, 40, 10, 0, 0, 0, 0, 0}, {36, 40, 10, 0, 0, 0, 0, 0}},          // neutral
  {{38, 38, 13, 0, -2, 0, 0, 20}, {38, 38, 13, 0, -2, 0, 0, 20}},      // happy: crescents
  {{34, 32, 10, 0, 5, 3, 15, 0}, {34, 32, 10, 0, 5, 3, 15, 0}},        // sad: outer corners droop
  {{36, 34, 7, 0, 3, 17, 3, 0}, {36, 34, 7, 0, 3, 17, 3, 0}},          // angry: lids slam to the nose
  {{40, 46, 20, 0, -3, 0, 0, 0}, {40, 46, 20, 0, -3, 0, 0, 0}},        // surprised: wide open
  {{38, 40, 9, 0, 6, 25, 25, 3}, {38, 40, 9, 0, 6, 25, 25, 3}},        // sleepy: heavy lids
  {{36, 40, 10, 0, 0, 0, 0, 0}, {36, 40, 10, 0, 0, 0, 0, 0}},          // love: hearts drawn on top
  {{38, 42, 15, 0, -3, 0, 0, 13}, {38, 42, 15, 0, -3, 0, 0, 13}},      // excited
  {{38, 38, 13, 0, -2, 0, 0, 18}, {38, 5, 2, 0, 4, 0, 0, 0}},          // wink
  {{36, 40, 10, 0, 0, 0, 0, 0}, {36, 40, 10, 0, 0, 0, 0, 0}},          // dizzy: spirals
  {{36, 40, 10, 0, 1, 21, 21, 7}, {36, 40, 10, 0, -1, 6, 8, 0}},       // suspicious
  {{40, 30, 6, 0, 0, 12, 12, 0}, {40, 30, 6, 0, 0, 12, 12, 0}},        // cool: shades drawn on top
  {{34, 36, 10, 0, 2, 9, 4, 0}, {40, 46, 16, 0, -3, 0, 0, 0}},         // curious: one big eye
  {{30, 34, 12, 0, 0, 0, 0, 0}, {30, 34, 12, 0, 0, 0, 0, 0}},          // scared: small + shiver
  {{38, 40, 8, 0, 4, 21, 21, 0}, {38, 40, 8, 0, 4, 21, 21, 0}},        // bored: flat half lids
  {{40, 30, 12, 0, -4, 0, 0, 19}, {40, 30, 12, 0, -4, 0, 0, 19}},      // laugh: squeezed crescents
  {{38, 30, 6, 0, 2, 13, 5, 5}, {38, 30, 6, 0, 2, 13, 5, 5}},          // determined: narrowed, focused
};

struct Face {
  Adafruit_SSD1306 d{OLED_W, OLED_H, &Wire, -1};
  bool ok = false;
  SemaphoreHandle_t mtx = nullptr;
  volatile uint8_t id = FC_NEUTRAL;   // what is on screen
  volatile uint8_t mood = FC_NEUTRAL; // what it returns to
  uint32_t idStart = 0, lastFrame = 0;
  float gx = 0, gy = 0, tgx = 0, tgy = 0;   // gaze (-1..1)
  EyeShape cur[2];
  uint32_t blinkAt = 0, blinkT = 0;
  bool doubleBlink = false;
  String text, title;
  uint32_t textUntil = 0;
  uint8_t bmp[1024];
  int8_t bmpSlot = -1;

  struct Lock {
    SemaphoreHandle_t m;
    Lock(SemaphoreHandle_t x) : m(x) { if (m) xSemaphoreTake(m, portMAX_DELAY); }
    ~Lock() { if (m) xSemaphoreGive(m); }
  };

  bool begin() {
    mtx = xSemaphoreCreateMutex();
    cur[0] = EYES[FC_NEUTRAL][0];
    cur[1] = EYES[FC_NEUTRAL][1];
    ok = d.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (!ok) return false;
    d.clearDisplay();
    d.setTextColor(SSD1306_WHITE);
    d.display();
    blinkAt = millis() + 2500;
    xTaskCreatePinnedToCore(task, "face", 4096, this, 1, nullptr, 0);
    return true;
  }

  static void task(void* p) {
    Face* f = (Face*)p;
    TickType_t last = xTaskGetTickCount();
    for (;;) {
      f->frame(millis());
      vTaskDelayUntil(&last, pdMS_TO_TICKS(40));   // 25 fps
    }
  }

  void show(uint8_t newId) {
    Lock l(mtx);
    if (newId >= FC_BMP0 && newId < FC_BMP0 + 8) {
      if (!loadSlot(newId - FC_BMP0)) newId = FC_NEUTRAL;
    }
    id = newId;
    idStart = millis();
  }
  void setMood(uint8_t m) { if (m >= FC_NEUTRAL && m <= FC_MOOD_LAST) { mood = m; show(m); } }
  void back() { show(mood); }
  void titleCard(const char* t) { { Lock l(mtx); title = t; } show(FC_TITLE); }
  void lookAt(float x, float y) { tgx = constrain(x, -1.0f, 1.0f); tgy = constrain(y, -1.0f, 1.0f); }
  void banner(const String& t, uint32_t ms) { Lock l(mtx); text = t; textUntil = millis() + ms; }
  bool isEyes() const { return id >= FC_NEUTRAL && id <= FC_MOOD_LAST; }

  static String slotPath(int slot) { return "/f" + String(slot) + ".bin"; }
  bool loadSlot(int slot) {          // caller holds the lock
    File f = LittleFS.open(slotPath(slot), "r");
    if (!f || f.size() != 1024) return false;
    f.read(bmp, 1024);
    f.close();
    bmpSlot = slot;
    return true;
  }
  bool slotExists(int slot) {
    File f = LittleFS.open(slotPath(slot), "r");
    bool e = f && f.size() == 1024;
    if (f) f.close();
    return e;
  }
  bool saveSlot(int slot, const uint8_t* data) {
    File f = LittleFS.open(slotPath(slot), "w");
    if (!f) return false;
    size_t w = f.write(data, 1024);
    f.close();
    return w == 1024;
  }
  void showRaw(const uint8_t* data) {   // Face Studio live preview (not saved)
    Lock l(mtx);
    memcpy(bmp, data, 1024);
    bmpSlot = -1;
    id = FC_BMP0;
    idStart = millis();
  }
  // Copy of the last frame in SSD1306 page order (remote's live mirror).
  void snapshot(uint8_t* out) { Lock l(mtx); memcpy(out, d.getBuffer(), 1024); }

  // ---------------------------------------------------------------- drawing helpers
  void heart(int x, int y, int r, uint16_t c = SSD1306_WHITE) {
    d.fillCircle(x - r, y, r, c);
    d.fillCircle(x + r, y, r, c);
    d.fillTriangle(x - 2 * r, y + 1, x + 2 * r, y + 1, x, y + 2 * r + 2, c);
  }
  void sparkle(int x, int y, int s, uint16_t c = SSD1306_WHITE) {
    d.drawFastHLine(x - s, y, 2 * s + 1, c);
    d.drawFastVLine(x, y - s, 2 * s + 1, c);
    if (s > 2) {
      d.drawLine(x - s / 2, y - s / 2, x + s / 2, y + s / 2, c);
      d.drawLine(x - s / 2, y + s / 2, x + s / 2, y - s / 2, c);
    }
  }
  void spiral(int cx, int cy, float rot, float scale = 0.36f) {
    for (int k = 0; k < 44; k++) {
      float a = k * 0.5f + rot, r = k * scale;
      d.drawPixel(cx + r * cosf(a), cy + r * sinf(a), SSD1306_WHITE);
    }
  }

  // One eye. inner = +1 when the nose is to the right of this eye (left eye), -1 for the right eye.
  void drawEye(float hx, float hy, const EyeShape& e, float open, float scale, int inner) {
    float sq = 1 + 0.10f * (1 - open);                // squash: a closing eye gets a little wider
    int w = (int)(e.w * scale * sq + 0.5f);
    int h = (int)(e.h * scale * open + 0.5f);
    if (h < 3) h = 3;
    int cx = (int)(hx + e.x), cy = (int)(hy + e.y);
    int x0 = cx - w / 2, y0 = cy - h / 2;
    int r = (int)min(e.r * scale, (float)min(w, h) / 2);
    d.fillRoundRect(x0, y0, w, h, r, SSD1306_WHITE);
    float k = (float)h / max(1.0f, e.h * scale);      // lids keep proportion while blinking
    int li = (int)(e.lidIn * k), lo = (int)(e.lidOut * k);
    int xl = x0 - 2, xr = x0 + w + 2, yt = y0 - 2;
    int yL = y0 + (inner > 0 ? lo : li), yR = y0 + (inner > 0 ? li : lo);
    if (li > 0 || lo > 0) {
      d.fillTriangle(xl, yt, xr, yt, xr, yR, SSD1306_BLACK);
      d.fillTriangle(xl, yt, xr, yR, xl, yL, SSD1306_BLACK);
    }
    int low = (int)(e.lidLow * k);
    if (low > 0) d.fillCircle(cx, y0 + h + w - low, w, SSD1306_BLACK);   // curved lower lid
  }

  void shades(int y) {   // "deal with it" sunglasses
    for (int e = 0; e < 2; e++) {
      int x = e ? 74 : 14;
      d.fillRoundRect(x, y, 40, 20, 6, SSD1306_WHITE);
      d.fillRect(x, y, 40, 4, SSD1306_WHITE);
      d.drawLine(x + 8, y + 15, x + 16, y + 6, SSD1306_BLACK);    // glint
      d.drawLine(x + 13, y + 15, x + 19, y + 9, SSD1306_BLACK);
    }
    d.fillRect(54, y + 2, 20, 4, SSD1306_WHITE);                  // bridge
    d.fillRect(0, y, 14, 3, SSD1306_WHITE);
    d.fillRect(114, y, 14, 3, SSD1306_WHITE);
  }

  void drawEyes(uint8_t m, uint32_t now, uint32_t t, float dt) {
    // morph toward the mood's shape (~70 ms time constant)
    float k = 1 - expf(-dt / 0.07f);
    for (int e = 0; e < 2; e++) {
      const EyeShape& T = EYES[m][e];
      float* c = &cur[e].w;
      const float* g = &T.w;
      for (int i = 0; i < 8; i++) c[i] += (g[i] - c[i]) * k;
    }
    // blink: fast close, slower open, sometimes twice
    float open = 1;
    if (blinkT) {
      uint32_t b = now - blinkT;
      if (b < 70) open = 1 - b / 70.0f;
      else if (b < 190) { float u = (b - 70) / 120.0f; open = u * u * (3 - 2 * u); }
      else if (doubleBlink) { doubleBlink = false; blinkT = now; open = 1; }
      else blinkT = 0;
    } else if (now >= blinkAt && m != FC_SLEEPY && m != FC_DIZZY && m != FC_WINK) {
      blinkT = now;
      doubleBlink = random(0, 6) == 0;
      blinkAt = now + random(2000, 5500);
    }
    // gaze with a quick saccade-like ease
    float kg = 1 - expf(-dt / 0.06f);
    gx += (tgx - gx) * kg;
    gy += (tgy - gy) * kg;
    float ox = gx * 10, oy = gy * 7;
    if (m == FC_SCARED) { ox += random(-1, 2); oy += random(-1, 2); }
    if (m == FC_SLEEPY) oy += 2 * sinf(t / 900.0f);
    float bob = m == FC_EXCITED ? 3 * sinf(t / 110.0f) : m == FC_LAUGH ? -3 * fabsf(sinf(t / 95.0f)) : 0;
    float sl = 1 - 0.10f * gx, sr = 1 + 0.10f * gx;   // perspective: the eye on the gaze side looks closer
    float lx = 32 + ox, rx = 96 + ox, cy = 32 + oy + bob;

    if (m == FC_LOVE) {
      int r = 9 + (int)(2 * sinf(t / 140.0f));
      heart((int)lx - 1, (int)cy - 5, r);
      heart((int)rx - 1, (int)cy - 5, r);
      return;
    }
    if (m == FC_DIZZY) { spiral((int)lx, (int)cy, t / 90.0f); spiral((int)rx, (int)cy, -t / 90.0f); return; }

    drawEye(lx, cy, cur[0], open, sl, +1);
    drawEye(rx, cy, cur[1], open, sr, -1);

    switch (m) {
      case FC_SAD: d.fillCircle((int)rx + 12, (int)cy + 14 + (t / 80) % 18, 2, SSD1306_WHITE); break;  // tear
      case FC_SLEEPY:
        d.setTextSize(1);
        d.setCursor(108, 14 - (t / 140) % 12); d.print("z");
        d.setCursor(116, 8 - (t / 190) % 8);   d.print("Z");
        break;
      case FC_EXCITED:
        sparkle((int)lx - 6, (int)cy - 9, 4, SSD1306_BLACK);
        sparkle((int)rx - 6, (int)cy - 9, 4, SSD1306_BLACK);
        break;
      case FC_COOL: shades((int)(cy - 12)); break;
      case FC_SCARED:   // sweat drop
        d.fillCircle(120, 14 + (t / 60) % 10, 3, SSD1306_WHITE);
        d.fillTriangle(117, 13 + (t / 60) % 10, 123, 13 + (t / 60) % 10, 120, 7 + (t / 60) % 10, SSD1306_WHITE);
        break;
      case FC_LAUGH: d.setTextSize(1); d.setCursor(46 + (int)(2 * sinf(t / 60.0f)), 55); d.print("HA HA"); break;
      case FC_CURIOUS: if ((t / 600) % 2) { d.setTextSize(2); d.setCursor(116, 0); d.print("?"); } break;
      default: break;
    }
  }

  void drawScreen(uint8_t s, uint32_t now, uint32_t t) {
    switch (s) {
      case FC_EQ:
        for (int i = 0; i < 16; i++) {
          int h = 4 + (int)((sinf(t / 110.0f + i * 0.7f) + sinf(t / 170.0f - i * 1.3f) + 2) * 12);
          d.fillRect(i * 8, 63 - h, 6, h, SSD1306_WHITE);
        }
        break;
      case FC_HEARTS:
        for (int i = 0; i < 6; i++) {
          int x = (i % 2 ? 100 : 8) + (i / 2) * 9 + (int)(sinf((t + i * 300) / 500.0f) * 4);
          int y = 70 - (int)((t / 18 + i * 37) % 84);
          heart(x, y, 3 + i % 2);
        }
        heart(64, 22, 9 + (int)(3 * sinf(t / 150.0f)));
        break;
      case FC_STARS:
        for (int i = 0; i < 8; i++)
          sparkle(10 + (i * 37) % 112, 8 + (i * 23) % 48, 1 + (int)(3 * fabsf(sinf((t + i * 250) / 300.0f))));
        break;
      case FC_SPIRAL:
        spiral(34, 32, t / 100.0f, 0.4f);
        spiral(94, 32, -t / 100.0f, 0.4f);
        break;
      case FC_GLITCH:
        d.setTextSize(3);
        d.setCursor(19 + random(-2, 3), 20 + random(-2, 3));
        d.print("ROBOT");
        for (int i = 0; i < 3; i++) d.fillRect(random(0, 128), random(0, 64), random(10, 50), 2, SSD1306_INVERSE);
        break;
      case FC_NOTES:
        for (int i = 0; i < 4; i++) {
          int x = 12 + i * 32, y = 60 - (int)((t / 14 + i * 29) % 64);
          d.fillCircle(x, y + 8, 4, SSD1306_WHITE);
          d.drawFastVLine(x + 3, y - 6, 15, SSD1306_WHITE);
          d.drawLine(x + 3, y - 6, x + 8, y - 2, SSD1306_WHITE);
        }
        break;
      case FC_MATRIX:
        for (int i = 0; i < 16; i++) {
          int x = i * 8, y = (int)((t / 20 + i * 53) % 90) - 20;
          for (int k = 0; k < 5; k++) d.drawFastHLine(x + 1, y - k * 5, 5 - (k > 2), SSD1306_WHITE);
        }
        break;
      case FC_TITLE: {   // dance title card
        d.setTextSize(1);
        d.setCursor(31, 4);
        d.print("now dancing");
        d.setTextSize(2);
        d.setCursor((128 - (int)title.length() * 12) / 2, 22);
        d.print(title);
        int u = umin(104, t / 4);
        d.fillRect(64 - u / 2, 42, u, 2, SSD1306_WHITE);
        for (int i = 0; i < 2; i++) {
          int x = i ? 114 : 10, y = 56 - (int)(7 * fabsf(sinf((t + i * 300) / 170.0f)));
          d.fillCircle(x, y, 3, SSD1306_WHITE);
          d.drawFastVLine(x + 2, y - 11, 11, SSD1306_WHITE);
          d.drawLine(x + 2, y - 11, x + 6, y - 8, SSD1306_WHITE);
        }
        break;
      }
      case FC_ECG: {     // heart monitor: a few beats, then flatline
        static const int8_t BLIP[10] = {0, -3, 0, 4, -24, 16, 0, -5, -3, 0};
        bool alive = t < 1700;
        int py = 40;
        for (int x = 0; x < 128; x++) {
          int ph = (x + (int)(t / 5)) % 44, y = 40;
          if (alive && ph < 10) y += BLIP[ph];
          if (x) d.drawLine(x - 1, py, x, y, SSD1306_WHITE);
          py = y;
        }
        d.setTextSize(1);
        d.setCursor(0, 0);
        if (alive) { heart(108, 6, 3); d.print("BPM 72"); }
        else { d.print("BPM 0"); d.setTextSize(2); d.setCursor(46, 10); d.print("x_x"); }
        break;
      }
      case FC_BOOT: {
        int letters = umin(6, t / 180);
        d.setTextSize(3);
        d.setCursor(10, 10);
        for (int i = 0; i < letters; i++) d.print("TROJAN"[i]);
        d.drawFastHLine(10, 38, umin(108, t / 6), SSD1306_WHITE);
        d.setTextSize(1);
        d.setCursor(30, 48);
        d.print("waking up");
        for (uint32_t k = 0; k < (t / 300) % 4; k++) d.print('.');
        break;
      }
      case FC_WIFI: {
        d.fillCircle(18, 24, 2, SSD1306_WHITE);
        int arcs = 1 + (t / 400) % 3;
        for (int r = 1; r <= arcs; r++)
          for (int a = -45; a <= 45; a += 3)
            d.drawPixel(18 + r * 6 * sinf(a * DEG_TO_RAD), 24 - r * 6 * cosf(a * DEG_TO_RAD), SSD1306_WHITE);
        d.setTextSize(2);
        d.setCursor(42, 4);
        d.print("WiFi");
        d.setTextSize(1);
        d.setCursor(0, 32);
        d.print(AP_SSID);
        d.setCursor(0, 42);
        d.print("Pass: " AP_PASS);
        d.setCursor(0, 54);
        d.print(WiFi.softAPIP().toString());
        break;
      }
      case FC_CONNECTED: {
        float p = min(1.0f, t / 500.0f);
        float q1 = min(1.0f, p / 0.4f);
        int bx = 40 + (int)(14 * q1), by = 22 + (int)(14 * q1);
        for (int o = 0; o < 3; o++) d.drawLine(40, 22 + o, bx, by + o, SSD1306_WHITE);
        if (p > 0.4f) {
          float q = (p - 0.4f) / 0.6f;
          for (int o = 0; o < 3; o++) d.drawLine(54, 36 + o, 54 + (int)(34 * q), 36 - (int)(30 * q) + o, SSD1306_WHITE);
        }
        d.setTextSize(2);
        d.setCursor(10, 46);
        d.print("CONNECTED");
        break;
      }
    }
  }

  // One frame: compose under the lock (fast), then push it over I2C.
  void frame(uint32_t now) {
    if (!ok) return;
    float dt = lastFrame ? min(0.2f, (now - lastFrame) / 1000.0f) : 0.04f;
    lastFrame = now;
    {
      Lock l(mtx);
      uint32_t t = now - idStart;
      d.clearDisplay();
      d.setTextColor(SSD1306_WHITE);
      d.setTextWrap(false);
      uint8_t cid = id;
      if (cid >= FC_BMP0 && cid < FC_BMP0 + 8) d.drawBitmap(0, 0, bmp, 128, 64, SSD1306_WHITE);
      else if (cid >= FC_NEUTRAL && cid <= FC_MOOD_LAST) drawEyes(cid, now, t, dt);
      else drawScreen(cid, now, t);
      if (textUntil) {
        if (now > textUntil) {
          textUntil = 0;
        } else {
          d.fillRect(0, 52, 128, 12, SSD1306_BLACK);
          d.setTextSize(1);
          int w = text.length() * 6;
          int x = w <= 124 ? (128 - w) / 2 : 128 - (int)((now / 25) % (w + 128));
          d.setCursor(x, 54);
          d.print(text);
        }
      }
    }
    d.display();
  }
};
