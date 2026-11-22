"""Draws the TROJAN doc diagrams as PNGs (PIL only). Output dir = argv[1]."""
import math, sys
from PIL import Image, ImageDraw, ImageFont

OUT = sys.argv[1]
S = 2  # supersample
NAVY, TEAL, ORANGE, GREY, LIGHT, INK, RED = "#1F3A5F", "#2A9D8F", "#E76F51", "#8A94A6", "#EEF2F7", "#1B1F24", "#C0392B"
F = "/System/Library/Fonts/Supplemental/Arial.ttf"
FB = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"


def font(sz, bold=False):
    return ImageFont.truetype(FB if bold else F, sz * S)


def canvas(w, h):
    im = Image.new("RGB", (w * S, h * S), "white")
    return im, ImageDraw.Draw(im)


def P(*xy):
    return [v * S for v in xy]


def box(d, x, y, w, h, fill=LIGHT, outline=NAVY, r=10, width=2):
    d.rounded_rectangle(P(x, y, x + w, y + h), radius=r * S, fill=fill, outline=outline, width=width * S)


def text(d, x, y, s, sz=14, bold=False, fill=INK, anchor="mm"):
    d.multiline_text((x * S, y * S), s, font=font(sz, bold), fill=fill, anchor=anchor, align="center", spacing=4 * S)


def arrow(d, x1, y1, x2, y2, col=NAVY, w=3, head=10, both=False):
    d.line(P(x1, y1, x2, y2), fill=col, width=w * S)
    def tip(ax, ay, bx, by):
        a = math.atan2(by - ay, bx - ax)
        pts = [(bx, by), (bx - head * math.cos(a - 0.4), by - head * math.sin(a - 0.4)),
               (bx - head * math.cos(a + 0.4), by - head * math.sin(a + 0.4))]
        d.polygon([(px * S, py * S) for px, py in pts], fill=col)
    tip(x1, y1, x2, y2)
    if both:
        tip(x2, y2, x1, y1)


def save(im, name):
    im = im.resize((im.width // S, im.height // S), Image.LANCZOS)
    im.save(f"{OUT}/{name}.png", dpi=(200, 200))


# 1. System block diagram
def block():
    im, d = canvas(1000, 430)
    box(d, 30, 60, 170, 90); text(d, 115, 105, "Phone / Laptop\nweb remote", 16, True)
    box(d, 30, 250, 170, 90); text(d, 115, 295, "Laptop\nUSB serial", 16, True)
    box(d, 380, 120, 230, 170, fill="#DDE7F3", r=14, width=3)
    text(d, 495, 165, "ESP32 DevKit", 20, True, NAVY)
    text(d, 495, 225, "TROJAN firmware 3.0\n50 Hz control loop\ngait + moves + faces", 13, False, INK)
    arrow(d, 200, 105, 378, 165, TEAL, both=True); text(d, 285, 95, "Wi-Fi\n\"Sesame-Controller\"", 12, False, TEAL, "md")
    arrow(d, 200, 295, 378, 250, GREY, both=True); text(d, 290, 290, "USB serial\n115200 baud", 12, False, GREY, "ma")
    box(d, 780, 40, 190, 80); text(d, 875, 80, "OLED 128x64\n(face)", 15, True)
    box(d, 780, 165, 190, 80); text(d, 875, 205, "Perfboard\nsignal + power hub", 15, True)
    box(d, 780, 290, 190, 80); text(d, 875, 330, "8 x MG90S\nservos", 15, True)
    arrow(d, 610, 160, 778, 85, NAVY); text(d, 640, 95, "I2C (GPIO 21/22)", 12, False, NAVY, "ms")
    arrow(d, 610, 205, 778, 205, NAVY); text(d, 695, 190, "8 PWM signals", 12, False, NAVY)
    arrow(d, 875, 245, 875, 288, NAVY)
    text(d, 500, 400, "Input  ->  ESP32 decides each servo angle 50 times a second  ->  legs move + face changes", 14, False, GREY)
    save(im, "d_block")


# 2. One leg: hip (top view) + foot (front view)
def leg():
    im, d = canvas(1000, 470)
    text(d, 250, 25, "HIP servo - seen from above", 17, True, NAVY)
    text(d, 750, 25, "FOOT servo - seen from the front", 17, True, NAVY)
    # top view: body corner + leg swinging
    box(d, 60, 170, 170, 130, fill="#DDE7F3", r=8); text(d, 145, 235, "body", 14, False, NAVY)
    cx, cy = 230, 235
    d.ellipse(P(cx - 14, cy - 14, cx + 14, cy + 14), fill=ORANGE)
    for ang, col, lab in ((-40, GREY, "forward"), (0, NAVY, "neutral"), (40, GREY, "back")):
        a = math.radians(ang)
        x2, y2 = cx + 190 * math.cos(a), cy + 190 * math.sin(a)
        d.line(P(cx, cy, x2, y2), fill=col, width=(9 if col == NAVY else 5) * S)
        text(d, x2 + 30, y2, lab, 12, False, col, "lm" if False else "mm")
    d.arc(P(cx - 120, cy - 120, cx + 120, cy + 120), -40, 40, fill=ORANGE, width=3 * S)
    text(d, 250, 400, "Hip turns the whole leg forward / back\n(about +-45 deg). This is what moves the robot.", 13, False, INK)
    # front view: body + upper leg + splaying lower leg
    bx, by = 560, 120
    box(d, bx, by, 120, 70, fill="#DDE7F3", r=8); text(d, bx + 60, by + 35, "body", 14, False, NAVY)
    kx, ky = bx + 190, by + 35
    d.line(P(bx + 120, ky, kx, ky), fill=NAVY, width=9 * S)
    d.ellipse(P(kx - 14, ky - 14, kx + 14, ky + 14), fill=ORANGE)
    for ang, col, lab in ((90, NAVY, "0 deg: foot down (stands tall)"), (60, GREY, "~25 deg: foot lifted (a step)"), (0, GREY, "90 deg: flat (lying down)")):
        a = math.radians(ang)
        x2, y2 = kx + 150 * math.cos(a), ky + 150 * math.sin(a)
        d.line(P(kx, ky, x2, y2), fill=col, width=(9 if col == NAVY else 5) * S)
        lx, ly, an = {90: (kx, ky + 172, "mm"), 60: (x2 + 12, y2, "lm"), 0: (kx + 30, ky - 22, "lm")}[ang]
        text(d, lx, ly, lab, 12, False, col, an)
    d.line(P(560, ky + 150, 900, ky + 150), fill=GREY, width=2 * S)
    text(d, 760, 400, "Foot splays the lower leg out to the side.\nSmall lift = step, big splay = body lowers.", 13, False, INK)
    save(im, "d_leg")


# 3. Top view with code names
def topview():
    im, d = canvas(800, 560)
    box(d, 300, 150, 200, 260, fill="#DDE7F3", r=16, width=3)
    text(d, 400, 230, "ESP32\n+ OLED face", 15, True, NAVY)
    arrow(d, 400, 140, 400, 60, ORANGE, 4); text(d, 400, 40, "FRONT (face)", 15, True, ORANGE)
    legs = [("FRONT RIGHT", "hip R1  /  foot R3", 500, 180, 1, -1), ("REAR RIGHT", "hip R2  /  foot R4", 500, 380, 1, 1),
            ("FRONT LEFT", "hip L1  /  foot L3", 300, 180, -1, -1), ("REAR LEFT", "hip L2  /  foot L4", 300, 380, -1, 1)]
    for name, codes, x, y, sx, sy in legs:
        ex, ey = x + sx * 130, y + sy * 70
        d.line(P(x, y, ex, ey), fill=NAVY, width=10 * S)
        d.ellipse(P(x - 13, y - 13, x + 13, y + 13), fill=ORANGE)
        d.ellipse(P(ex - 11, ey - 11, ex + 11, ey + 11), fill=TEAL)
        tx = ex + sx * 10
        text(d, tx, ey + sy * 38, name, 14, True, NAVY, "rm" if sx < 0 else "lm")
        text(d, tx, ey + sy * 60, codes, 13, False, INK, "rm" if sx < 0 else "lm")
    text(d, 400, 530, "orange = hip servo     teal = foot servo     (robot's own right is on the right)", 12, False, GREY)
    save(im, "d_topview")


# 4. Power wiring
def power():
    im, d = canvas(1000, 500)
    box(d, 30, 60, 210, 90, fill="#FDEBE5", outline=ORANGE); text(d, 135, 105, "LiPo A  3.7 V\n1300 mAh", 15, True, ORANGE)
    box(d, 30, 320, 210, 90, fill="#FDEBE5", outline=ORANGE); text(d, 135, 365, "LiPo B  3.7 V\n1300 mAh", 15, True, ORANGE)
    box(d, 380, 50, 220, 110, fill="#DDE7F3", r=14, width=3); text(d, 490, 105, "ESP32 DevKit", 17, True, NAVY)
    box(d, 760, 50, 200, 90); text(d, 860, 95, "OLED", 16, True)
    box(d, 380, 300, 220, 130, fill=LIGHT, r=14, width=3); text(d, 490, 340, "Perfboard (Atharv)", 15, True, NAVY)
    text(d, 490, 385, "+ rail | GND rail | 8 signal\nheaders, no buck converter", 12, False, INK)
    box(d, 760, 310, 200, 110); text(d, 860, 365, "8 x MG90S\nservos", 16, True)
    arrow(d, 240, 100, 378, 100, RED); text(d, 310, 85, "+ to 5V/VIN", 12, False, RED)
    arrow(d, 240, 360, 378, 360, RED); text(d, 310, 345, "+ to rail", 12, False, RED)
    arrow(d, 600, 80, 758, 80, RED); text(d, 680, 65, "3.3 V", 12, False, RED)
    arrow(d, 600, 115, 758, 115, NAVY); text(d, 680, 132, "SDA 21 / SCL 22", 12, False, NAVY)
    arrow(d, 520, 160, 520, 298, TEAL); text(d, 600, 230, "8 PWM signal\nwires (GPIO)", 12, False, TEAL)
    arrow(d, 600, 365, 758, 365, NAVY); text(d, 680, 350, "red/brown/orange", 12, False, NAVY)
    arrow(d, 240, 135, 378, 135, INK); text(d, 310, 150, "GND", 12, True, INK)
    arrow(d, 240, 395, 378, 395, INK); text(d, 310, 410, "GND", 12, True, INK)
    arrow(d, 440, 160, 440, 298, INK); text(d, 425, 230, "GND", 12, True, INK, "rm")
    text(d, 500, 475, "COMMON GROUND: both batteries, the ESP32 and every servo meet on the perfboard GND rail", 13, True, INK)
    save(im, "d_power")


# 5. Gait timing
def gait():
    im, d = canvas(1000, 400)
    text(d, 500, 22, "Which legs are in the air (orange) during one step cycle", 16, True, NAVY)
    rows = ["Front right", "Rear left", "Front left", "Rear right"]
    def chart(x0, title, spans):
        text(d, x0 + 210, 60, title, 15, True, INK)
        for i, r in enumerate(rows):
            y = 90 + i * 55
            text(d, x0 + 70, y + 18, r, 12, False, INK, "rm")
            d.rectangle(P(x0 + 80, y, x0 + 420, y + 36), fill=LIGHT, outline=GREY, width=1 * S)
            a, b = spans[i]
            d.rectangle(P(x0 + 80 + 340 * a, y, x0 + 80 + 340 * b, y + 36), fill=ORANGE)
    chart(20, "CRAWL: one leg at a time, 3 always down", [(0, .25), (.25, .5), (.5, .75), (.75, 1)])
    chart(520, "TROT: diagonal pairs together", [(0, .45), (0, .45), (.5, .95), (.5, .95)])
    text(d, 500, 335, "Crawl = slow but very stable.  Trot = faster; the two feet left on the ground are\n"
         "diagonal, so the body balances on a line through its centre.", 13, False, GREY)
    save(im, "d_gait")


for f in (block, leg, topview, power, gait):
    f()
print("ok")
