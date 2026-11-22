#pragma once
// OLED face engine: animated eyes with moods, procedural screens, user-drawn bitmaps, text banner.
// Nothing is stored as bitmaps except what the user draws in Face Studio (LittleFS /f0.bin .. /f7.bin).
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "config.h"

// Face ids: 1..12 moods, 32..38 animated screens, 48..50 system screens, 64..71 custom bitmaps.
enum : uint8_t {
  FC_NEUTRAL = 1, FC_HAPPY, FC_SAD, FC_ANGRY, FC_SURPRISED, FC_SLEEPY, FC_LOVE, FC_EXCITED, FC_WINK, FC_DIZZY, FC_SUSPICIOUS, FC_COOL,
  FC_EQ = 32, FC_HEARTS, FC_STARS, FC_SPIRAL, FC_GLITCH, FC_NOTES, FC_MATRIX,
  FC_BOOT = 48, FC_WIFI, FC_CONNECTED,
  FC_BMP0 = 64
};

static inline uint32_t umin(uint32_t a, uint32_t b) { return a < b ? a : b; }

struct Face {
  Adafruit_SSD1306 d{OLED_W, OLED_H, &Wire, -1};
  bool ok = false;
  uint8_t id = FC_NEUTRAL;      // what is on screen
  uint8_t mood = FC_NEUTRAL;    // what it returns to
  uint32_t idStart = 0, lastDraw = 0;
  float gx = 0, gy = 0, tgx = 0, tgy = 0;   // gaze (-1..1)
  uint32_t blinkAt = 0, blinkT = 0;
  bool autoBlink = true;
  String text;
  uint32_t textUntil = 0;
  uint8_t bmp[1024];
  int8_t bmpSlot = -1;

  bool begin() {
    ok = d.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (!ok) return false;
    d.clearDisplay();
    d.setTextColor(SSD1306_WHITE);
    d.display();
    blinkAt = millis() + 2500;
    return true;
  }

  void show(uint8_t newId) {
    if (newId >= FC_BMP0 && newId < FC_BMP0 + 8) {
      if (!loadSlot(newId - FC_BMP0)) newId = FC_NEUTRAL;
    }
    id = newId;
    idStart = millis();
    lastDraw = 0;
  }
  void setMood(uint8_t m) { if (m >= FC_NEUTRAL && m <= FC_COOL) { mood = m; show(m); } }
  void back() { show(mood); }
  void lookAt(float x, float y) { tgx = constrain(x, -1.0f, 1.0f); tgy = constrain(y, -1.0f, 1.0f); }
  void banner(const String& t, uint32_t ms) { text = t; textUntil = millis() + ms; lastDraw = 0; }
  bool isEyes() const { return id >= FC_NEUTRAL && id <= FC_COOL; }

  static String slotPath(int slot) { return "/f" + String(slot) + ".bin"; }
  bool loadSlot(int slot) {
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
  // Show raw bytes without saving (Face Studio live preview).
  void showRaw(const uint8_t* data) {
    memcpy(bmp, data, 1024);
    bmpSlot = -1;
    id = FC_BMP0;
    idStart = millis();
    lastDraw = 0;
  }

  // ---------------------------------------------------------------- drawing
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
  void eye(int cx, int cy, int w, int h, int r) {
    if (h < 3) { d.fillRect(cx - w / 2, cy - 1, w, 3, SSD1306_WHITE); return; }
    d.fillRoundRect(cx - w / 2, cy - h / 2, w, h, min(r, h / 2), SSD1306_WHITE);
  }

  void drawEyes(uint8_t m, uint32_t now, uint32_t t) {
    // blink
    float open = 1;
    if (blinkT) {
      uint32_t e = now - blinkT;
      if (e < 160) open = fabsf(cosf(PI * e / 160.0f)); else blinkT = 0;
    } else if (autoBlink && now >= blinkAt && m != FC_SLEEPY && m != FC_DIZZY) {
      blinkT = now;
      blinkAt = now + random(2200, 6000);
    }
    gx += (tgx - gx) * 0.35f;
    gy += (tgy - gy) * 0.35f;
    int ox = (int)(gx * 9), oy = (int)(gy * 6);
    int cl = 34 + ox, cr = 94 + ox, cy = 30 + oy;
    const int W = 34, H = 40;
    int h = (int)(H * open);

    switch (m) {
      case FC_NEUTRAL:
        eye(cl, cy, W, h, 11);
        eye(cr, cy, W, h, 11);
        break;
      case FC_HAPPY:  // crescent eyes: a circle with a lower circle cut out
        for (int e = 0; e < 2; e++) {
          int cx = e ? cr : cl;
          d.fillCircle(cx, cy + 6, 17, SSD1306_WHITE);
          d.fillCircle(cx, cy + 16, 17, SSD1306_BLACK);
        }
        break;
      case FC_SAD:
        eye(cl, cy + 2, W - 4, h - 4, 10);
        eye(cr, cy + 2, W - 4, h - 4, 10);
        d.fillTriangle(cl - 20, cy - 24, cl + 20, cy - 24, cl - 20, cy + 2, SSD1306_BLACK);  // droopy outer lids
        d.fillTriangle(cr - 20, cy - 24, cr + 20, cy - 24, cr + 20, cy + 2, SSD1306_BLACK);
        d.fillCircle(cr + 10, cy + 14 + (t / 90) % 14, 2, SSD1306_WHITE);                     // tear
        break;
      case FC_ANGRY:
        eye(cl, cy + 2, W, h - 4, 8);
        eye(cr, cy + 2, W, h - 4, 8);
        d.fillTriangle(cl - 20, cy - 24, cl + 20, cy - 24, cl + 20, cy + 4, SSD1306_BLACK);  // lids slant to the nose
        d.fillTriangle(cr - 20, cy - 24, cr + 20, cy - 24, cr - 20, cy + 4, SSD1306_BLACK);
        break;
      case FC_SURPRISED:
        for (int e = 0; e < 2; e++) {
          int cx = e ? cr : cl;
          d.fillCircle(cx, cy, 19, SSD1306_WHITE);
          d.fillCircle(cx, cy, 13, SSD1306_BLACK);
          d.fillCircle(cx + ox / 2, cy + oy / 2, 5, SSD1306_WHITE);
        }
        break;
      case FC_SLEEPY: {
        int sh = 6 + (int)(4 * (0.5f + 0.5f * sinf(t / 700.0f)));
        eye(cl, cy + 6, W, sh, 4);
        eye(cr, cy + 6, W, sh, 4);
        d.setTextSize(1);
        d.setCursor(104, 6 + (t / 120) % 10);
        d.print("z");
        d.setCursor(112, 2 + (t / 160) % 8);
        d.print("Z");
        break;
      }
      case FC_LOVE: {
        int r = 9 + (int)(2 * sinf(t / 140.0f));
        heart(cl - 1, cy - 4, r);
        heart(cr - 1, cy - 4, r);
        break;
      }
      case FC_EXCITED: {
        int bob = (int)(3 * sinf(t / 110.0f));
        eye(cl, cy + bob, W + 2, H, 12);
        eye(cr, cy + bob, W + 2, H, 12);
        sparkle(cl - 5, cy - 8 + bob, 4, SSD1306_BLACK);
        sparkle(cr - 5, cy - 8 + bob, 4, SSD1306_BLACK);
        break;
      }
      case FC_WINK:
        eye(cl, cy, W, h, 11);
        d.fillRect(cr - 14, cy, 28, 4, SSD1306_WHITE);
        d.fillRect(cr - 14, cy - 3, 4, 3, SSD1306_WHITE);
        d.fillRect(cr + 10, cy - 3, 4, 3, SSD1306_WHITE);
        break;
      case FC_DIZZY:
        spiral(cl, cy, t / 90.0f);
        spiral(cr, cy, -t / 90.0f);
        break;
      case FC_SUSPICIOUS:
        eye(cl, cy, W, h, 11);
        eye(cr, cy, W, h, 11);
        d.fillRect(cl - 20, cy - 24, 40, 24, SSD1306_BLACK);   // left eye half shut
        d.fillRect(cr - 20, cy - 24, 40, 8, SSD1306_BLACK);    // right eye barely lidded
        break;
      case FC_COOL:
        eye(cl, cy, W + 4, 30, 8);
        eye(cr, cy, W + 4, 30, 8);
        d.fillRect(cl - 22, cy - 15, 120, 11, SSD1306_BLACK);   // visor
        d.fillRect(cl - 22, cy - 5, 120, 1, SSD1306_WHITE);
        break;
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

  void tick(uint32_t now) {
    if (!ok) return;
    bool animated = id < FC_BMP0;
    uint32_t period = (id >= FC_EQ && id <= FC_MATRIX) ? 70 : 85;
    if (!animated && !textUntil) period = 400;
    if (now - lastDraw < period) return;
    lastDraw = now;
    uint32_t t = now - idStart;
    d.clearDisplay();
    d.setTextColor(SSD1306_WHITE);
    d.setTextWrap(false);
    if (id >= FC_BMP0 && id < FC_BMP0 + 8) {
      d.drawBitmap(0, 0, bmp, 128, 64, SSD1306_WHITE);
    } else if (isEyes()) {
      drawEyes(id, now, t);
    } else {
      drawScreen(id, now, t);
    }
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
    d.display();
  }
};
