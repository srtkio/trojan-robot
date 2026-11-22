"""Builds the TROJAN mini-project documentation .docx. argv: img_dir out_path"""
import sys
from docx import Document
from docx.enum.section import WD_SECTION
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_CELL_VERTICAL_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_BREAK
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Pt, Inches, RGBColor, Cm

IMG, OUT = sys.argv[1], sys.argv[2]
NAVY, TEAL, ORANGE, GREY, INK = "1F3A5F", "2A9D8F", "E76F51", "6B7280", "1B1F24"
LIGHT, ZEBRA = "EEF2F7", "F7F9FC"
FONT = "Calibri"
rgb = RGBColor.from_string

doc = Document()
sec = doc.sections[0]
sec.page_width, sec.page_height = Inches(8.27), Inches(11.69)  # A4
for side in ("left_margin", "right_margin"):
    setattr(sec, side, Inches(0.9))
sec.top_margin, sec.bottom_margin = Inches(0.8), Inches(0.8)
CONTENT_W = 8.27 - 1.8


# ---------- styles ----------
def style_font(st, size, bold=False, color=INK, italic=False):
    st.font.name = FONT
    st.element.rPr.rFonts.set(qn("w:eastAsia"), FONT)
    st.font.size = Pt(size)
    st.font.bold = bold
    st.font.italic = italic
    st.font.color.rgb = rgb(color)


normal = doc.styles["Normal"]
style_font(normal, 10.5)
normal.paragraph_format.space_after = Pt(6)
normal.paragraph_format.line_spacing = 1.15

h1 = doc.styles["Heading 1"]
style_font(h1, 18, True, NAVY)
h1.paragraph_format.space_before = Pt(20)
h1.paragraph_format.space_after = Pt(8)
h1.paragraph_format.keep_with_next = True
h2 = doc.styles["Heading 2"]
style_font(h2, 13, True, TEAL)
h2.paragraph_format.space_before = Pt(14)
h2.paragraph_format.space_after = Pt(4)
h2.paragraph_format.keep_with_next = True
for name in ("List Bullet", "List Number"):
    style_font(doc.styles[name], 10.5)
    doc.styles[name].paragraph_format.space_after = Pt(3)


def set_cell_fill(cell, hexcol):
    tcPr = cell._tc.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), hexcol)
    tcPr.append(shd)


def set_borders(table, color="D5DCE6", size=4, inside=True):
    tblPr = table._tbl.tblPr
    b = OxmlElement("w:tblBorders")
    for edge in ("top", "left", "bottom", "right", "insideH", "insideV"):
        e = OxmlElement(f"w:{edge}")
        if not inside and edge.startswith("inside") or color is None:
            e.set(qn("w:val"), "nil")
        else:
            e.set(qn("w:val"), "single")
            e.set(qn("w:sz"), str(size))
            e.set(qn("w:color"), color)
        b.append(e)
    tblPr.append(b)


def cell_margins(table, top=70, bottom=70, left=110, right=110):
    tblPr = table._tbl.tblPr
    m = OxmlElement("w:tblCellMar")
    for k, v in (("top", top), ("bottom", bottom), ("left", left), ("right", right)):
        e = OxmlElement(f"w:{k}")
        e.set(qn("w:w"), str(v))
        e.set(qn("w:type"), "dxa")
        m.append(e)
    tblPr.append(m)


def para_border_bottom(p, color=TEAL, size=12):
    pPr = p._p.get_or_add_pPr()
    bdr = OxmlElement("w:pBdr")
    e = OxmlElement("w:bottom")
    e.set(qn("w:val"), "single")
    e.set(qn("w:sz"), str(size))
    e.set(qn("w:space"), "4")
    e.set(qn("w:color"), color)
    bdr.append(e)
    pPr.append(bdr)


def add_runs(p, text, size=None, color=None, bold=None, italic=None):
    """Text with **bold** segments."""
    parts = text.split("**")
    for i, part in enumerate(parts):
        if not part:
            continue
        r = p.add_run(part)
        if i % 2:
            r.bold = True
        if bold is not None and not i % 2:
            r.bold = bold
        if size:
            r.font.size = Pt(size)
        if color:
            r.font.color.rgb = rgb(color)
        if italic:
            r.italic = True
    return p


def P(text="", size=None, color=None, align=None, after=None, italic=None, bold=None):
    p = doc.add_paragraph()
    add_runs(p, text, size, color, bold, italic)
    if align:
        p.alignment = align
    if after is not None:
        p.paragraph_format.space_after = Pt(after)
    return p


def H1(text):
    p = doc.add_heading(text, 1)
    para_border_bottom(p)
    return p


def H2(text):
    return doc.add_heading(text, 2)


def bullets(items, style="List Bullet"):
    for it in items:
        add_runs(doc.add_paragraph(style=style), it)


def QA(q, a):
    p = P(q, color=NAVY, bold=True, after=2)
    p.paragraph_format.keep_with_next = True
    if isinstance(a, list):
        bullets(a)
    else:
        P(a)


def fix_widths(t, widths):
    """Fixed layout + explicit grid so Word, Pages and Google Docs all keep the column widths."""
    t.autofit = False
    tblPr = t._tbl.tblPr
    lay = OxmlElement("w:tblLayout")
    lay.set(qn("w:type"), "fixed")
    tblPr.append(lay)
    tw = OxmlElement("w:tblW")
    tw.set(qn("w:w"), str(int(sum(widths) * 1440)))
    tw.set(qn("w:type"), "dxa")
    for old in tblPr.findall(qn("w:tblW")):
        tblPr.remove(old)
    tblPr.append(tw)
    for gc, w in zip(t._tbl.tblGrid.findall(qn("w:gridCol")), widths):
        gc.set(qn("w:w"), str(int(w * 1440)))
    for row in t.rows:
        for c, w in zip(row.cells, widths):
            c.width = Inches(w)


def table(headers, rows, widths, first_col_bold=True, size=9.5):
    hdr = headers is not None
    headers = headers or [""] * len(widths)
    t = doc.add_table(rows=1 + len(rows), cols=len(headers))
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    set_borders(t)
    cell_margins(t)
    fix_widths(t, widths)
    for r_i, row in enumerate([headers] + rows):
        for c_i, val in enumerate(row):
            c = t.cell(r_i, c_i)
            c.width = Inches(widths[c_i])
            c.vertical_alignment = WD_CELL_VERTICAL_ALIGNMENT.CENTER
            p = c.paragraphs[0]
            p.paragraph_format.space_after = Pt(0)
            p.paragraph_format.line_spacing = 1.1
            if r_i == 0 and hdr:
                set_cell_fill(c, NAVY)
                add_runs(p, val, size, "FFFFFF", True)
            else:
                if r_i % 2 == 0 and hdr:
                    set_cell_fill(c, ZEBRA)
                add_runs(p, val, size, NAVY if (c_i == 0 and first_col_bold) else INK, True if (c_i == 0 and first_col_bold) else None)
    # repeat header row on page breaks
    if not hdr:
        t._tbl.remove(t.rows[0]._tr)
        doc.add_paragraph().paragraph_format.space_after = Pt(2)
        return t
    trPr = t.rows[0]._tr.get_or_add_trPr()
    h = OxmlElement("w:tblHeader")
    h.set(qn("w:val"), "true")
    trPr.append(h)
    doc.add_paragraph().paragraph_format.space_after = Pt(2)
    return t


def callout(text, fill=LIGHT, bar=TEAL):
    t = doc.add_table(rows=1, cols=1)
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    set_borders(t, None)
    cell_margins(t, 110, 110, 180, 180)
    fix_widths(t, [CONTENT_W])
    c = t.cell(0, 0)
    set_cell_fill(c, fill)
    tcPr = c._tc.get_or_add_tcPr()
    b = OxmlElement("w:tcBorders")
    left = OxmlElement("w:left")
    left.set(qn("w:val"), "single")
    left.set(qn("w:sz"), "24")
    left.set(qn("w:color"), bar)
    b.append(left)
    tcPr.append(b)
    p = c.paragraphs[0]
    p.paragraph_format.space_after = Pt(0)
    add_runs(p, text, 10)
    doc.add_paragraph().paragraph_format.space_after = Pt(2)


FIG = [0]


def figure(items, height=2.6, width=None):
    """items: list of (file, caption). Side by side, borderless."""
    t = doc.add_table(rows=1, cols=len(items))
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    set_borders(t, None)
    fix_widths(t, [CONTENT_W / len(items)] * len(items))
    for i, (f, cap) in enumerate(items):
        c = t.cell(0, i)
        p = c.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_after = Pt(2)
        run = p.add_run()
        if width:
            run.add_picture(f"{IMG}/{f}", width=Inches(width))
        else:
            run.add_picture(f"{IMG}/{f}", height=Inches(height))
        FIG[0] += 1
        cp = c.add_paragraph()
        cp.alignment = WD_ALIGN_PARAGRAPH.CENTER
        cp.paragraph_format.space_after = Pt(4)
        r = cp.add_run(f"Figure {FIG[0]}. ")
        r.bold, r.font.size, r.font.color.rgb = True, Pt(8.5), rgb(NAVY)
        r = cp.add_run(cap)
        r.italic, r.font.size, r.font.color.rgb = True, Pt(8.5), rgb(GREY)
    doc.add_paragraph().paragraph_format.space_after = Pt(0)


def page_number_field(p):
    r = p.add_run()
    for kind, txt in (("begin", None), (None, "PAGE"), ("end", None)):
        if kind:
            e = OxmlElement("w:fldChar")
            e.set(qn("w:fldCharType"), kind)
        else:
            e = OxmlElement("w:instrText")
            e.set(qn("xml:space"), "preserve")
            e.text = txt
        r._r.append(e)
    r.font.size, r.font.color.rgb = Pt(8.5), rgb(GREY)


# ---------- cover ----------
sec.different_first_page_header_footer = True
for _ in range(3):
    P("", after=0)
P("ROBOTICS CLUB  ·  MINI PROJECT", 10, TEAL, bold=True, after=4)
P("TROJAN", 46, NAVY, bold=True, after=0)
P("Sesame Spiderbot — Project Documentation", 16, GREY, after=14)
p = P("", after=18)
para_border_bottom(p, ORANGE, 18)
figure([("s_final_happy.jpg", "TROJAN running its own firmware (v3), Nov 2026")], height=4.2)
table(None, [
    ["Team", "Trojans"],
    ["Members", "Sarthak Rai (lead) · Atharv · Antriksh · Shaurya"],
    ["Robot", "TROJAN — 8-servo ESP32 quadruped, built from the open-source Sesame Robot"],
    ["Project dates", "30 Oct 2026 → 10 Nov 2026 (v1 complete) · firmware improved to v3 by 18 Nov"],
], [1.5, CONTENT_W - 1.5])
doc.add_page_break()

# header / footer (pages after the cover)
hp = sec.header.paragraphs[0]
hp.alignment = WD_ALIGN_PARAGRAPH.RIGHT
add_runs(hp, "TROJAN  ·  Team Trojans  ·  Sesame Spiderbot documentation", 8.5, GREY)
fp = sec.footer.paragraphs[0]
fp.alignment = WD_ALIGN_PARAGRAPH.CENTER
page_number_field(fp)

# ---------- 1 ----------
H1("1. Team & project details")
table(["Field", "Our answer"], [
    ["Team name", "Trojans"],
    ["Robot name", "TROJAN"],
    ["Mentor", "[add mentor name]"],
    ["Start → completion", "30 Oct 2026 → 10 Nov 2026 (version 1 walking). Firmware kept improving to v3.0 (18 Nov 2026)."],
    ["Firmware used", "Started with the official sesame-firmware-main (adapted to our pins). Final: TROJAN firmware 3.0, our own rewrite."],
    ["Board used", "Plain ESP32 DevKit (30-pin, ESP-WROOM-32) + our own perfboard instead of a Distro Board"],
    ["Code", "TROJAN_firmware_v3.zip (submitted with this doc)  ·  [add GitHub link if uploaded]"],
    ["Walking video", "5_WALKING_VIDEO_final_Nov11.mp4 (submitted with this doc)  ·  [add Drive/YouTube link]"],
], [1.7, CONTENT_W - 1.7])

table(["Member", "Primary area", "What they built / did"], [
    ["Sarthak Rai", "Lead · mechanical + firmware", "Printed and assembled the robot, led the build, and wrote the TROJAN firmware and phone remote."],
    ["Atharv", "Electronics", "Designed and soldered the compact perfboard: shared power and ground rails for all 8 servos and one signal header per motor."],
    ["Antriksh", "Calibration & alignment", "Centred every servo before the horns went on, mapped which motor moves which joint, and tuned leg angles so TROJAN walks straight."],
    ["Shaurya", "Wiring, debugging & testing", "Wired the ESP32, OLED and servos, traced faults (wrong pins, loose jumpers, the burnt OLEDs) and ran the test checklist."],
], [1.2, 1.6, CONTENT_W - 2.8])
figure([("p_team.jpg", "Building day in the lab")], height=2.8)

H2("Why this project")
P("A **quadruped** is a robot that walks on four legs. It is more stable than a two-legged robot and can step over "
  "small bumps that stop wheels. The club picked **Sesame** as a first project because it is small, cheap, fully 3D "
  "printed and open source, and it uses one ESP32 and eight hobby servos, so one project covers mechanical design, "
  "electronics and code. We wanted to learn how to wire and power a real robot, how to program an ESP32 over Wi-Fi, "
  "and how walking actually works.")

# ---------- 2 ----------
H1("2. Understanding the original repo")
table(["Repo folder / file", "What it contains", "Did we use or change it?"], [
    ["hardware/", "CAD files, STL print files, bill of materials and printing guide", "Used — printed all body and leg parts from the STLs"],
    ["docs/", "Build guide and wiring guide with photos", "Used for the assembly order; our wiring differs (own perfboard, two batteries)"],
    ["sesame-firmware-main.ino", "The main program: Wi-Fi hotspot, web server, servo setup, commands, serial CLI", "Used first with our pins, then replaced by our own firmware"],
    ["movement-sequences.h", "Every pose and walk as a list of fixed servo angles", "Used first; replaced by a maths-based walking engine"],
    ["face-bitmaps.h", "OLED face pictures stored as pixel arrays", "Used first; replaced by faces drawn in code"],
    ["captive-portal.h", "The control web page the phone opens", "Used first; replaced by our new 4-tab remote"],
    ["motor-testing/", "How to test each motor from the Serial Monitor (e.g. typing 0,90)", "Used the idea: tested every servo alone before assembly"],
    ["software/", "Links to Sesame Studio (pose/face editor) and a browser simulator", "Read only; we built our own Pose Lab and Face Studio into the remote"],
], [1.75, 2.45, CONTENT_W - 4.2])

QA("How the whole system works", "The phone (or laptop) joins the robot's own Wi-Fi and opens a web page. Each button or "
   "joystick move sends a small message to the ESP32. The ESP32 works out the angle every servo should be at, 50 times a "
   "second, and sends each servo a PWM signal through the perfboard. At the same time it draws TROJAN's face on the OLED "
   "over I2C. A laptop can also send the same commands over the USB cable (serial).")
figure([("d_block.png", "System block diagram")], width=6.2)
QA("Which resources we studied first", "The main README, the build-guide video, the wiring guide and the firmware README. "
   "Most confusing: which pin block to use for a bare ESP32 DevKit (we had no Distro Board), and why the left-side "
   "servos need \"opposite\" angles to the right side.")
QA("Which board and why", "A plain ESP32 DevKit. We already had it, it has built-in Wi-Fi and plenty of free pins, and "
   "the firmware supports it. Instead of a Distro Board, Atharv made a small perfboard that does the same job: power "
   "and ground for the servos plus a header for each signal.")
QA("What the Apache-2.0 license allows", "We may use, change, share and even sell the design. If we publish a fork we "
   "must keep the license file and the original copyright notices, say which files we changed, and not use the author's "
   "name to suggest he endorses our version.")

# ---------- 3 ----------
H1("3. Mechanical design")
H2("3.1 Legs and movement")
QA("One leg and its 2 servos", ["The **hip servo** turns the whole leg forward and back around the body corner (about ±45°). This is what pushes the robot along.",
                                "The **foot servo** splays the lower leg out to the side. A small lift (about 25°) raises the foot for a step; a big splay lowers the whole body."])
figure([("d_leg.png", "What each servo in a leg does")], width=6.3)
QA("Servos and degrees of freedom", "8 servos, 2 per leg, so 8 degrees of freedom (DOF). Each servo is one joint that can move on its own.")
QA("Leg names used in the code", "The printed parts carry the same names as the code. Front legs: R1/R3 and L1/L3. Rear legs: R2/R4 and L2/L4 (the first is the hip, the second the foot).")
figure([("d_topview.png", "Top view with code names"), ("p_plate_labels.jpg", "Real body plate, labels printed on the parts")], height=2.9)
QA("Why 180° servos and not 360°", "A 180° servo is told an angle and holds it, even under load, so a leg stays where we put it. "
   "A 360° \"continuous\" servo only gets a speed and direction; it cannot hold a position, so the legs would just spin and the robot would collapse.")

H2("3.2 Assembly")
QA("Order we assembled it in", ["Printed the body, legs and covers in PLA.",
                                "Tested each MG90S on its own and set it to 90° before fitting the horn.",
                                "Mounted the four hip servos in the frame, then attached each leg with its foot servo.",
                                "Fitted the body plate, the ESP32 on top, the OLED in the head and the perfboard.",
                                "Strapped the two batteries under the body with zip ties, then calibrated every joint."])
figure([("p_frame.jpg", "Frame with the four hip servos"), ("p_leg.jpg", "One leg: MG90S + printed links"), ("p_assembled.jpg", "Assembled, top view")], height=2.3)
QA("How we set every servo to 90° first, and why", "We ran a small test sketch (and later the serial command \"servo 90\") to "
   "drive each servo to 90°, then pushed the horn on pointing in the leg's neutral direction. The code assumes 90° is the "
   "middle of every joint. If a horn is fitted one tooth off, that leg is offset, the robot leans and it drifts while walking.")
QA("Problems we faced", ["At first we **skipped centring** the servos. The legs moved to random positions and fought each other until we centred all of them and refitted the horns.",
                         "The body was designed for different parts than ours, so the **batteries did not fit**. We hung them under the body with zip ties.",
                         "All wiring was **jumper wires** with no soldering, and the top shell is only as big as the ESP32, so it was crowded and wires got pulled loose.",
                         "Feet slipped on smooth tables, so we added **rubber grips** to all four feet."])
figure([("p_calibration.jpg", "Calibration notes: rest angle of every motor"), ("p_early_wiring.jpg", "Early wiring: jumper wires everywhere")], height=2.9)

# ---------- 4 ----------
H1("4. Electronics & power")
H2("4.1 What each part does")
table(["Component", "What it does", "Connected to"], [
    ["ESP32 DevKit", "The brain: runs the firmware, makes the Wi-Fi hotspot, sends PWM to every servo", "Servo GPIOs 14, 27, 13, 12, 33, 32, 25, 26 · OLED 21/22 · battery A"],
    ["Servos ×8 (MG90S)", "Metal-gear micro servos, one per joint", "Signal → its GPIO through the perfboard · power from the battery B rail · common GND"],
    ["OLED 0.96\" 128×64", "Shows TROJAN's face, moods and Wi-Fi info", "SDA GPIO 21, SCL GPIO 22, VCC 3.3 V, GND"],
    ["Batteries", "2 × 3.7 V single-cell LiPo, 1300 mAh, 25C", "A → ESP32 (and OLED through it) · B → servo power rail"],
    ["Perfboard (no buck converter)", "Atharv's board: shared + and GND rails for the servos and 8 signal headers", "Jumper wires to the ESP32 pins and both batteries"],
], [1.55, 2.4, CONTENT_W - 3.95])
figure([("p_esp32.jpg", "ESP32 DevKit we used"), ("p_perf_front.jpg", "Perfboard: 8 servo headers"), ("p_perf_back.jpg", "Back: soldered power and GND rails")], height=2.5)

H2("4.2 Pin map")
table(["Motor (index)", "Leg / joint", "GPIO", "Tested"], [
    ["0", "R1 — front right hip", "14", "✓"], ["1", "R2 — rear right hip", "27", "✓"],
    ["2", "L1 — front left hip", "13", "✓"], ["3", "L2 — rear left hip", "12", "✓"],
    ["4", "R4 — rear right foot", "33", "✓"], ["5", "R3 — front right foot", "32", "✓"],
    ["6", "L3 — front left foot", "25", "✓"], ["7", "L4 — rear left foot", "26", "✓"],
    ["OLED SDA", "—", "21", "✓"], ["OLED SCL", "—", "22", "✓"],
], [1.3, 2.6, 1.2, CONTENT_W - 5.1])

H2("4.3 Power")
figure([("d_power.png", "Power and signal wiring")], width=6.3)
QA("The 3 wires on a servo", "Brown = ground, red = power (+), orange = signal. The signal is a stream of pulses from the ESP32 that tells the servo which angle to go to.")
QA("Battery and why about 3 A", "Two 3.7 V single-cell LiPos, 1300 mAh each: one only for the ESP32, one only for the servos. "
   "One MG90S draws little when resting but several hundred mA when it pushes or stalls. When all 8 lift the body at once the total peak is around 3 A.")
figure([("p_batteries.jpg", "The two 3.7 V 1300 mAh LiPos, strapped under the body"), ("p_oled_test.jpg", "OLED test before fitting it")], height=2.7)
QA("What a buck converter does", "It steps a higher battery voltage (for example 7.4 V) down to a steady 5 V for the servos. "
   "We did not use one: the servos run straight off a 3.7 V cell. That works, but it is below the MG90S's 4.8–6 V rating, "
   "so the servos are weaker and slower than they could be.")
QA("Brownouts", "A brownout is when the supply voltage dips for a moment (for example when all servos start together) and the ESP32 "
   "resets. Our main protection is the separate battery for the ESP32, so servo current spikes only pull down battery B. "
   "Firmware v3 also records why the ESP32 last restarted (brownout or crash), so a brownout can be spotted from the phone.")
QA("Why all grounds must be connected", "A signal is a voltage measured against ground. If the ESP32 and the servos do not share "
   "ground, the servo cannot read the pulse properly and twitches or does not move. We learned this the hard way: some "
   "motors did not get their PWM until the common ground rail on the perfboard was in place.")

# ---------- 5 ----------
H1("5. Embedded firmware")
callout("**Two firmwares.** We first ran the official Sesame firmware, adapted to our pins and safe angle limits. Once the "
        "hardware worked, we wrote our own firmware from scratch (TROJAN 1.0 → 2.0 → 3.0) with a new walking engine and "
        "remote. The questions below are answered for the official code, with how TROJAN 3.0 does it where it differs.")
H2("5.1 Uploading the code")
table(["Item", "What we used"], [
    ["Board in Arduino IDE", "ESP32 Dev Module (esp32 by Espressif)"],
    ["Libraries", "ESP32Servo, Adafruit GFX, Adafruit SSD1306 (WiFi, WebServer, DNSServer come with the ESP32 core)"],
    ["Pin block", "The ESP32 DevKit block: servoPins = {14, 27, 13, 12, 33, 32, 25, 26}; OLED on SDA 21 / SCL 22"],
    ["Upload problem + fix", "Uploading itself was fine. The real early problems were wiring: wrong pin mapping (including the burnt OLEDs) and missing common ground. See 7.2."],
], [1.7, CONTENT_W - 1.7])

H2("5.2 What each file does")
table(["Official file", "What it does"], [
    ["sesame-firmware-main.ino", "Starts everything (Wi-Fi, web server, OLED, servos), runs the main loop, and has setServoAngle() and the serial CLI"],
    ["movement-sequences.h", "All poses and moves (stand, walk, wave, dance…) as fixed angle lists with short delays between them"],
    ["face-bitmaps.h", "Pictures for the OLED face, one pixel array per expression and animation frame"],
    ["captive-portal.h", "The HTML/JavaScript control page, stored in the code and sent to the phone"],
], [1.9, CONTENT_W - 1.9])
table(["TROJAN 3.0 file", "What it does"], [
    ["TROJAN_Robot_v3.ino", "setup() and a loop that runs the robot 50 times a second"],
    ["config.h", "Measured hardware facts: pins, safe angle limits, leg layout"],
    ["robot.h", "Turns leg angles into raw servo angles, with limits and a speed cap"],
    ["gait.h", "The walking engine: crawl, trot and run, any direction plus turning"],
    ["anim.h", "Tricks and dances calculated smoothly instead of jumping between poses"],
    ["face.h", "Eyes, moods and animations drawn in code"],
    ["ctrl.h", "Decides what the robot is doing: driving, a trick, idle, sleep"],
    ["web.h · remote.h · fastserver.h", "The phone remote page and the fast web server behind it"],
], [1.9, CONTENT_W - 1.9])

H2("5.3 How the code works")
QA("setup() and loop()", "setup() runs once: it starts the serial port, the I2C bus for the OLED, attaches the 8 servos "
   "(500–2400 µs pulses), starts the Wi-Fi hotspot, the captive-portal DNS and the web server, and shows a face. "
   "loop() runs forever: it answers web requests, animates the face, reads serial commands and runs the current command. "
   "In TROJAN 3.0, loop() also runs a 20 ms tick that recalculates all 8 angles every time.")
QA("Pressing \"Forward\" on the phone", ["The page sends a small web request to the robot (192.168.4.1).",
                                         "The web server's handler stores the command \"forward\".",
                                         "loop() sees it and calls runWalkPose().",
                                         "runWalkPose() calls setServoAngle() for pairs of servos, frame by frame.",
                                         "setServoAngle() writes the angle; the ESP32 sends the matching PWM pulse and the servo turns.",
                                         "TROJAN 3.0: the joystick sends speed, sideways and turn values several times a second, and the gait engine turns them into leg angles. If messages stop for 0.5 s the robot stops by itself (dead-man safety)."], )
QA("setServoAngle()", "It adds that motor's small trim, clamps the angle to that motor's safe range (for example R1 can only "
   "go 80–180°, so a leg can never hit the body), writes it to the servo, then waits motorCurrentDelay.")
QA("What PWM is", "Pulse-width modulation: the ESP32 sends a pulse every 20 ms (50 per second). The pulse length sets the angle: "
   "about 0.5 ms = 0°, about 1.45 ms = 90°, 2.4 ms = 180°. The servo's own circuit compares this with its position sensor and turns until they match.")
QA("runStandPose()", "Hips: R1 135°, R2 45°, L1 45°, L2 135°, so each leg points 45° out diagonally. Feet: R4 0°, R3 180°, L3 0°, L4 180°, so the feet go down and lift the body. "
   "Left and right are mirrored because the servos on each side are mounted facing opposite ways, so the same leg motion needs the opposite angle (180° − x).")
QA("Which legs move together", "In the official walk the hips move in diagonal pairs: R1 with L2 (front-right + rear-left), then "
   "R2 with L1 (rear-right + front-left). Two diagonal feet on the ground keep the body balanced on a line through its middle, like a trotting dog. "
   "TROJAN 3.0 can also crawl (one leg at a time, three always down), which is slower but very stable.")
figure([("d_gait.png", "Crawl vs trot: which legs are in the air")], width=6.3)
QA("frameDelay and walkCycles", "frameDelay (100 ms) is the pause between frames of a move, and walkCycles (10) is how many "
   "steps one Forward press makes. A smaller frameDelay makes the steps quicker, but past a point the feet slip and the walk "
   "gets messy. In TROJAN 3.0 these became the Speed slider and stride length in the remote.")
QA("motorCurrentDelay", "A 20 ms pause after each servo write, so the 8 servos do not all start at the same instant. That spreads "
   "out the current spikes and helps prevent brownouts. TROJAN 3.0 does this more smoothly by capping how fast any servo may turn.")
QA("Open-loop or closed-loop?", "Open-loop. The robot sends angles and trusts that they happen; it has no sensor to tell if it "
   "slipped, tilted or fell. A closed-loop robot measures (for example with a tilt sensor, an IMU) and corrects itself. "
   "This is why our drift had to be fixed by tuning rather than automatically.")
QA("How the phone connects", "The ESP32 makes its own Wi-Fi network, \"Sesame-Controller\" (default password 12345678). "
   "After joining, the control page opens by itself (captive portal), or go to 192.168.4.1 or trojan.local. "
   "TROJAN 3.0 allows one driver at a time, and the password can be changed from the remote; the current one is shown on the robot's screen at start-up.")
figure([("s_wifi_screen.jpg", "OLED showing the Wi-Fi name, password and address")], height=2.4)

# ---------- 6 ----------
H1("6. Decisions & changes")
H2("A. Why the repo does it this way")
table(["Choice in the repo", "Why we think the author did it"], [
    ["2 servos per leg", "The fewest motors that can still walk: one moves the leg forward/back, one lifts it. Fewer motors = cheaper, lighter, less current."],
    ["motorCurrentDelay between servo moves", "Starting 8 servos at once pulls a big current spike that can reset the ESP32. Small gaps spread the load."],
    ["Robot makes its own Wi-Fi", "Works anywhere with no router or internet; the phone connects straight to the robot and the page opens automatically."],
    ["Legs move in diagonal pairs", "Two diagonal feet on the ground keep the body balanced while the other two step."],
], [2.0, CONTENT_W - 2.0])
H2("B. What we changed")
table(["What we changed", "Why", "Worked?"], [
    ["Own perfboard + two separate batteries, no buck converter", "No Distro Board available; a separate battery keeps servo spikes away from the ESP32", "Yes. Servos are weaker at 3.7 V"],
    ["Our pin map and a safe angle range per motor", "Our wiring differs; the limits stop legs from hitting the body", "Yes"],
    ["Rewrote the firmware from scratch (TROJAN 1.0 → 3.0)", "The official walk drifted. We measured our real leg geometry and wrote a maths-based gait", "Yes. 1.0 walked straight on its first run"],
    ["Fixed the drift", "Found the front legs stepped twice as far as the rear ones, plus uneven lift timing. Equal strides, steering trim and per-leg tuning", "Yes"],
    ["New gaits: crawl, trot, run + joystick driving", "Smooth control in any direction at any speed", "Yes"],
    ["All moves calculated 50× a second", "Jumping between poses was jerky; MG90S also twitch on tiny slow moves, so those were removed", "Yes, much smoother"],
    ["New 4-tab phone remote", "Drive, Play (18 tricks, 8 dances), Face, More (Pose Lab, dance builder, servo test, tuning)", "Yes"],
    ["New faces: eyes with pupils, moods, idle personality", "The original faces were static pictures", "Yes"],
    ["Faster web server, one driver at a time", "The stock server served one phone at a time and paused the servos, which caused random stalls", "Yes, stalls gone"],
    ["Rubber grips on the feet", "Feet slipped on smooth tables", "Yes"],
], [2.05, 3.0, CONTENT_W - 5.05])
callout("The firmware rewrite was led by Sarthak with help from an AI coding assistant (Claude). The team tested every version on the robot and reported back what to fix.", fill="FDF3EF", bar=ORANGE)
figure([("s_first_stand.jpg", "1 Nov: first stand on the official firmware"), ("s_faces.jpg", "9 Nov: testing new faces"), ("s_final_walk2.jpg", "11 Nov: TROJAN walking on its own firmware")], height=2.5)

# ---------- 7 ----------
H1("7. Testing & problems")
H2("7.1 Test checklist")
table(["Test", "Result"], [
    ["Each servo moves on its own (Serial Monitor / web servo test)", "✓"],
    ["All servos set to 90° before fixing the horns", "✓  (after our first mistake, see 3.2)"],
    ["OLED shows a face", "✓"],
    ["Phone connects to the robot's Wi-Fi and the control page opens", "✓"],
    ["Robot stands without legs sagging", "✓"],
    ["Walks forward and backward, turns left and right", "✓"],
    ["At least 3 poses work (wave, dance, bow…)", "✓  18 tricks and 8 dances"],
    ["Runs on battery for a few minutes without resetting", "✓"],
], [4.6, CONTENT_W - 4.6], first_col_bold=False)
H2("7.2 Problems and fixes")
table(["Problem", "What caused it", "How we fixed it"], [
    ["Servos moved randomly at first", "Servos were not centred before the horns went on", "Centred every servo at 90°, refitted the horns, wrote down each motor's rest angle"],
    ["Burnt two OLEDs", "Wrong mapping: VCC and GND were swapped", "Check the pin labels before powering, colour-code wires, test the OLED alone first"],
    ["Some motors did not respond", "Loose jumper wires, no common ground, low voltage", "Perfboard with a common GND rail; tested every pin one by one"],
    ["Batteries did not fit, wiring crowded", "The body was designed for different parts; the top shell is ESP32-sized", "Zip-tied the batteries under the body. Next time: choose parts first, then design"],
    ["Walk drifted left or right", "Front legs stepped twice as far as the rear, uneven lift timing, servo offsets", "Measured each leg, rewrote the gait, added steering trim and rubber grips"],
    ["Random stalls while driving", "Web server waited on idle phones and blocked the servo loop", "Faster server with short timeouts; the remote retries cleanly"],
    ["Servos twitched during slow moves", "MG90S jitter on tiny angle steps", "Removed slow body \"breathing\"; idle life moved to the face"],
], [1.75, 2.15, CONTENT_W - 3.9])

# ---------- 8 ----------
H1("8. What we learned")
table(["Member", "What we learned"], [
    ["Sarthak Rai", "My first project with messy real electronics. How an ESP32 talks over Wi-Fi, how to write and structure code for it, how to wire a robot, and how to debug step by step."],
    ["Atharv", "Planning and soldering a perfboard, laying out power and ground rails, and why every part needs a common ground."],
    ["Antriksh", "How servos and PWM work, why centring matters, and how a few degrees of error in one leg makes the whole robot drift."],
    ["Shaurya", "Always check a pinout before powering (the OLED lesson), debug one thing at a time, and use the Serial Monitor to see what the board is doing."],
], [1.3, CONTENT_W - 1.3])
QA("Hardest part", "Getting TROJAN to walk straight. It kept drifting left or right, and some motors did not get a proper signal. "
   "We got past it by fixing the wiring (common ground, perfboard), measuring every leg's real movement, and rewriting the walking code around those measurements.")
QA("What we would add next", ["A **tilt sensor (IMU)** so the robot can feel when it leans and correct itself (closed-loop).",
                              "A **distance sensor** on the head to stop before obstacles.",
                              "A proper **PCB, a 2-cell battery and a 5 V buck converter**, so the servos get full power and the wiring is tidy.",
                              "Longer term, a bigger 12-servo robot dog (3 joints per leg) is already being planned as TROJAN v4."])

# ---------- 9 ----------
H1("9. Check your understanding")
table(["Topic", "Question", "Our answer"], [
    ["Mechanical", "How many servos and DOF?", "8 servos, 2 per leg, 8 DOF"],
    ["Mechanical", "Why 180° servos and not 360°?", "A 180° servo holds an angle; a 360° one only spins"],
    ["Mechanical", "Why set servos to 90° before fixing horns?", "90° is the code's middle; otherwise every leg is offset (we learned this first-hand)"],
    ["Mechanical", "Legs sag when standing. What could you change?", "More voltage for the servos (ours run at 3.7 V), lighter batteries, different stand angles"],
    ["Electronics", "The three servo wires?", "Brown GND, red power, orange signal"],
    ["Electronics", "Why about 3 A?", "8 servos lifting together draw large current peaks"],
    ["Electronics", "Brownout, and how the code reduces it?", "Voltage dip resets the ESP32; motorCurrentDelay spaces out moves. We also gave the ESP32 its own battery"],
    ["Electronics", "What does a buck converter do?", "Steps a higher voltage down to a steady 5 V"],
    ["Electronics", "One leg twitches randomly. How to find the cause?", "Swap the servo or the pin, check jumpers and common ground, check the battery"],
    ["Firmware", "Which file holds the poses and walks?", "movement-sequences.h (TROJAN 3.0: gait.h and anim.h)"],
    ["Firmware", "What does setServoAngle() do?", "Clamps to safe limits and sends the PWM for that angle"],
    ["Firmware", "What is PWM for a servo?", "A pulse every 20 ms; its width (0.5–2.4 ms) sets the angle"],
    ["Firmware", "frameDelay and walkCycles?", "Time between frames; number of steps per command"],
    ["Firmware", "Open-loop or closed-loop?", "Open-loop: no sensor feedback"],
    ["Firmware", "How would you add a new pose?", "Write a function with the servo angles and link it to a button (TROJAN 3.0: build it in the Pose Lab)"],
    ["Wi-Fi", "How does the phone connect?", "Join \"Sesame-Controller\"; the page opens or go to 192.168.4.1"],
    ["Gait", "Why diagonal pairs?", "Keeps the body balanced on two diagonal feet"],
    ["Gait", "Why does the robot drift?", "Uneven strides or offsets between legs, slippery floor. Ours: front legs stepped twice as far as the rear"],
], [1.05, 2.45, CONTENT_W - 3.5], size=9)

H2("Videos submitted with this doc (sound removed)")
bullets(["**5_WALKING_VIDEO_final_Nov11.mp4**: the walking video (TROJAN firmware)",
         "1_first_stand_stock_firmware_Nov01.mp4: first stand on the official firmware",
         "2_leg_calibration_top_view_Nov09.mp4: leg calibration from above",
         "3_standing_wifi_screen_Nov09.mp4: standing, OLED showing the Wi-Fi details",
         "4_faces_and_moves_test_Nov09.mp4: testing faces and moves"])

doc.core_properties.title = "TROJAN — Sesame Spiderbot Project Documentation"
doc.core_properties.author = "Team Trojans"
doc.save(OUT)
print("saved", OUT, "figures:", FIG[0])
