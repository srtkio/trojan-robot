"""Sesame-MG995: parametric 8-servo quadruped (CadQuery).

Axes: X forward, Y left, Z up. Units mm.
Run:  python sesame_mg995.py   -> writes STEP + STL into ./out

Every dimension marked VERIFY must be checked against your actual parts
(calipers) before printing.
"""
import math
import os

import cadquery as cq

# ---------------------------------------------------------------- MG995 (VERIFY)
SV_L, SV_W, SV_H = 40.7, 19.7, 36.5   # body: length (x_s), width (y_s), height to top (z_s<0)
SV_SHAFT_BACK = 10.0                  # shaft centre to nearest body end along x_s
SV_FL_LEN = 54.1                      # flange tab-to-tab length
SV_FL_T = 2.5                         # flange thickness
SV_FL_TOP = 9.0                       # flange top face, below top of body (z_s = -9)
SV_HOLE_DX = 48.5                     # flange hole spacing along x_s
SV_HOLE_DY = 5.0                      # flange hole offset +/- along y_s
SV_HOLE_D = 4.2                       # clearance hole for the bolt
SV_BOSS_D, SV_BOSS_H = 12.0, 3.4
SV_SPLINE_D, SV_SPLINE_TOP = 5.8, 6.4  # output spline top, above servo top
HORN_T = 3.0                          # plastic horn thickness on the spline
HORN_PCD_R = 9.0                      # horn screw circle radius (VERIFY)
HORN_SCREW_D = 2.4
HORN_CENTRE_D = 6.5                   # clears the centre screw head
SV_CUT_X = SV_L + 0.5                 # pocket sizes with print clearance
SV_CUT_Y = SV_W + 0.5

# ---------------------------------------------------------------- body
BODY_L, BODY_W, BODY_H = 220.0, 100.0, 46.0
WALL, FLOOR = 3.0, 3.0
HIP_X = 90.0                          # |x| of the hip shaft
HIP_Z = BODY_H / 2.0                  # shaft height
HIP_SHAFT_Y = BODY_W / 2.0 - WALL + SV_FL_TOP   # y of servo top = 56
LIPO_L, LIPO_W, LIPO_T = 76.0, 36.0, 21.0       # per 1300 mAh pack (VERIFY)
MEZZ_Z, MEZZ_T = 26.0, 3.0
GRID = 10.0                           # sensor grid pitch

# ---------------------------------------------------------------- legs
L1 = 80.0                             # hip axis -> knee axis
L2 = 100.0                            # knee axis -> foot tip
THIGH_T = 5.0
SHIN_T = 6.0
SPLINE_GAP = SV_SPLINE_TOP + 0.3      # servo top -> plate inner face
THIGH_Y = HIP_SHAFT_Y + SPLINE_GAP    # y of thigh inner face
SHIN_Z = SV_FL_TOP + SPLINE_GAP       # knee: servo top above thigh inner face
STAGGER = 28.0                        # rear legs sit this far outboard of the front legs


def box(cx, cy, cz, dx, dy, dz):
    return cq.Workplane("XY").box(dx, dy, dz).translate((cx, cy, cz))


def cyl_z(x, y, z0, z1, d):
    return (cq.Workplane("XY").workplane(offset=z0).center(x, y)
            .circle(d / 2).extrude(z1 - z0))


# ---------------------------------------------------------------- servo
SV_BODY_XC = -SV_SHAFT_BACK + SV_L / 2   # body centre along x_s (shaft at 0)
SV_HOLES_X = (SV_BODY_XC - SV_HOLE_DX / 2, SV_BODY_XC + SV_HOLE_DX / 2)


def mg995():
    """Reference servo: shaft axis = +Z at the origin, body hangs below z=0."""
    body = box(SV_BODY_XC, 0, -SV_H / 2, SV_L, SV_W, SV_H)
    flange = box(SV_BODY_XC, 0, -SV_FL_TOP - SV_FL_T / 2, SV_FL_LEN, SV_W, SV_FL_T)
    boss = cyl_z(0, 0, 0, SV_BOSS_H, SV_BOSS_D)
    spline = cyl_z(0, 0, SV_BOSS_H, SV_SPLINE_TOP, SV_SPLINE_D)
    s = body.union(flange).union(boss).union(spline)
    for hx in SV_HOLES_X:
        for hy in (-SV_HOLE_DY, SV_HOLE_DY):
            s = s.cut(cyl_z(hx, hy, -SV_FL_TOP - SV_FL_T - 1, -SV_FL_TOP + 1, SV_HOLE_D))
    return s


def servo_cutter(z_lo, z_hi):
    """Pocket + bolt holes for a servo whose flange top sits at z_s=-9.
    The plate/wall occupies z_s in [z_lo, z_hi]."""
    c = box(SV_BODY_XC, 0, (z_lo + z_hi) / 2, SV_CUT_X, SV_CUT_Y, (z_hi - z_lo) + 2)
    for hx in SV_HOLES_X:
        for hy in (-SV_HOLE_DY, SV_HOLE_DY):
            c = c.union(cyl_z(hx, hy, z_lo - 3, z_hi + 3, SV_HOLE_D))
    return c


def at(shape, origin, x_dir, normal):
    return shape.val().moved(cq.Location(cq.Plane(origin, x_dir, normal)))


# ---------------------------------------------------------------- body parts
def hip_plane(side, end):
    """side +1 left / -1 right, end +1 front / -1 rear.
    Servo body extends toward the body centre, shaft points outward."""
    origin = (end * HIP_X, side * HIP_SHAFT_Y, HIP_Z)
    return origin, (-end, 0, 0), (0, side, 0)


def body_base():
    outer = box(0, 0, BODY_H / 2, BODY_L, BODY_W, BODY_H)
    inner = box(0, 0, FLOOR + (BODY_H - FLOOR) / 2, BODY_L - 2 * WALL,
                BODY_W - 2 * WALL, BODY_H - FLOOR + 0.01)
    b = outer.cut(inner)

    # heat-set bosses for the top plate (inside the walls, above the servos)
    boss_pts = [(sx * x, sy * (BODY_W / 2 - WALL - 3.5))
                for x in (BODY_L / 2 - WALL - 3.5, 45.0, 0.0)
                for sx in ((1, -1) if x else (1,)) for sy in (1, -1)]
    for (x, y) in boss_pts:
        b = b.union(cyl_z(x, y, BODY_H - 12, BODY_H, 7.0))
        b = b.cut(cyl_z(x, y, BODY_H - 9, BODY_H + 1, 4.0))

    # mezzanine posts (M3 heat-set) and battery bay ribs
    for sx in (1, -1):
        for sy in (1, -1):
            b = b.union(cyl_z(sx * 44, sy * 42, FLOOR, MEZZ_Z, 8.0))
            b = b.cut(cyl_z(sx * 44, sy * 42, MEZZ_Z - 8, MEZZ_Z + 1, 4.0))
    b = b.union(box(0, 0, FLOOR + 6, LIPO_L + 4, 2.0, 12))            # divider
    for sx in (1, -1):
        for sy in (1, -1):
            b = b.union(box(sx * (LIPO_L / 2 + 1.5), sy * 18.5, FLOOR + 4, 2.0, 28, 8))

    # hip servo pockets + bolt holes
    z_lo, z_hi = -SV_FL_TOP, -SV_FL_TOP + WALL
    for side in (1, -1):
        for end in (1, -1):
            o, xd, n = hip_plane(side, end)
            cutter = servo_cutter(z_lo, z_hi)
            b = b.cut(cq.Workplane("XY").add(at(cutter, o, xd, n)))

    # rear wall: USB / charge window + power switch
    b = b.cut(box(-BODY_L / 2, -20, 20, 2 * WALL + 2, 14, 8))
    b = b.cut(box(-BODY_L / 2, 20, 20, 2 * WALL + 2, 20, 14))
    return b


def _grid(plate, xs, ys, z, t, d=3.2):
    for x in xs:
        for y in ys:
            plate = plate.cut(cyl_z(x, y, z - 1, z + t + 1, d))
    return plate


def mezzanine():
    p = box(0, 0, MEZZ_Z + MEZZ_T / 2, 100, 92, MEZZ_T)
    for sx in (1, -1):
        for sy in (1, -1):
            p = p.cut(cyl_z(sx * 44, sy * 42, MEZZ_Z - 1, MEZZ_Z + MEZZ_T + 1, 3.4))
    xs = [i * GRID for i in range(-4, 5)]
    ys = [i * GRID for i in range(-3, 4)]
    p = _grid(p, xs, ys, MEZZ_Z, MEZZ_T)
    for sy in (1, -1):
        p = p.cut(box(0, sy * 40, MEZZ_Z + MEZZ_T / 2, 40, 6, MEZZ_T + 2))
    return p


def top_plate():
    z0 = BODY_H
    t = 3.0
    p = box(0, 0, z0 + t / 2, BODY_L, BODY_W, t)
    xs = [i * GRID for i in range(-9, 10)]
    ys = [i * GRID for i in range(-4, 5)]
    p = _grid(p, xs, ys, z0, t)
    for (x, y) in [(sx * x, sy * (BODY_W / 2 - WALL - 3.5))
                   for x in (BODY_L / 2 - WALL - 3.5, 45.0, 0.0)
                   for sx in ((1, -1) if x else (1,)) for sy in (1, -1)]:
        p = p.cut(cyl_z(x, y, z0 - 1, z0 + t + 1, 3.4))
    p = p.cut(box(BODY_L / 2 - 14, 0, z0 + t / 2, 26.5, 14.5, t + 2))   # OLED window
    for sy in (1, -1):                                                  # cable/velcro slots
        p = p.cut(box(0, sy * 44, z0 + t / 2, 60, 5, t + 2))
    return p


# ---------------------------------------------------------------- legs (canonical frame)
# Plate lies in XY, hip axis at origin, leg hangs toward -Y, plate normal +Z.
def _horn_holes(plate, z0, z1):
    plate = plate.cut(cyl_z(0, 0, z0 - 1, z1 + 1, HORN_CENTRE_D))
    for k in range(4):
        a = math.radians(45 + 90 * k)
        plate = plate.cut(cyl_z(HORN_PCD_R * math.cos(a), HORN_PCD_R * math.sin(a),
                                z0 - 1, z1 + 1, HORN_SCREW_D))
    return plate


def thigh():
    t = THIGH_T
    p = cyl_z(0, 0, 0, t, 26.0)
    top = -L1 + SV_L - SV_SHAFT_BACK + 8
    p = p.union(box(0, (top) / 2, t / 2, 18, -top, t))               # bar hub -> servo frame
    y_lo = -L1 - SV_SHAFT_BACK - 8
    p = p.union(box(0, (y_lo + top) / 2, t / 2, 32, top - y_lo, t))  # servo frame
    # knee servo: body extends toward +Y (x_s -> +Y), shaft points +Z
    cut = servo_cutter(-SV_FL_TOP, -SV_FL_TOP + t)
    loc = cq.Location(cq.Plane((0, -L1, SV_FL_TOP), (0, 1, 0), (0, 0, 1)))
    p = p.cut(cq.Workplane("XY").add(cut.val().moved(loc)))
    return _horn_holes(p, 0, t)


def shin():
    t = SHIN_T
    p = cyl_z(0, 0, 0, t, 24.0)
    bar = (cq.Workplane("XY").polyline([(-8, 0), (8, 0), (5, -L2), (-5, -L2)])
           .close().extrude(t))
    p = p.union(bar)
    p = p.union(box(0, -(L2 + 12) / 2 - 0, t + 2, 3, L2 - 12, 4))    # stiffening rib
    foot = (cq.Workplane("XZ").workplane(offset=-(-L2 + 14)).center(0, t / 2)
            .circle(7).extrude(14))                                  # y from -L2+14 down to -L2
    p = p.union(foot)
    sock = (cq.Workplane("XZ").workplane(offset=-(-L2 + 15)).center(0, t / 2)
            .circle(4).extrude(15))
    p = p.cut(sock)
    return _horn_holes(p, 0, t)


def hip_spacer():
    """Hub that carries a rear thigh STAGGER mm outboard of the horn."""
    s = cyl_z(0, 0, 0, STAGGER + 0.01, 26.0)
    return _horn_holes(s, 0, STAGGER)


def leg_canonical(theta=0.0, phi=0.0, rear=False):
    """Returns (thigh, knee servo, shin) in the leg frame, posed.
    Rear legs are moved outboard by STAGGER so they clear the front leg's swing."""
    dz = STAGGER if rear else 0.0
    th = thigh().val().moved(cq.Location(cq.Vector(0, 0, dz)))
    if rear:
        th = th.fuse(hip_spacer().val())
    sv = mg995().val().moved(cq.Location(cq.Plane((0, -L1, SV_FL_TOP + dz), (0, 1, 0), (0, 0, 1))))
    sh = shin().val().moved(cq.Location(cq.Vector(0, -L1, SHIN_Z + dz)))
    sh = sh.rotate(cq.Vector(0, -L1, 0), cq.Vector(0, -L1, 1), phi)
    parts = [x.rotate(cq.Vector(0, 0, 0), cq.Vector(0, 0, 1), theta) for x in (th, sv, sh)]
    return parts


def place_leg(parts, side, end):
    """Move canonical leg to a hip. side +1 left (normal +Y) / -1 right."""
    origin = (end * HIP_X, side * THIGH_Y, HIP_Z)
    xdir = (-1, 0, 0) if side > 0 else (1, 0, 0)
    loc = cq.Location(cq.Plane(origin, xdir, (0, side, 0)))
    return [p.moved(loc) for p in parts]


# ---------------------------------------------------------------- assembly / checks
def assemble(theta=0.0, phi=0.0):
    out = []
    for side in (1, -1):
        for end in (1, -1):
            o, xd, n = hip_plane(side, end)
            out.append(at(mg995(), o, xd, n))
            out.extend(place_leg(leg_canonical(theta * end * side, phi, rear=end < 0), side, end))
    return out


def clearance_report(base):
    """Volume of leg parts overlapping the body / neighbours over the swing range."""
    worst = 0.0
    bad = []
    body = base.val()
    for theta in range(-60, 61, 15):
        for phi in range(-60, 61, 30):
            parts = place_leg(leg_canonical(theta, phi), 1, 1)
            for p in parts:
                v = body.intersect(p).Volume()
                worst = max(worst, v)
                if v > 1.0:
                    bad.append((theta, phi, round(v, 1)))
    return worst, bad


def export(name, wp_or_shape, out):
    shape = wp_or_shape.val() if hasattr(wp_or_shape, "val") else wp_or_shape
    cq.exporters.export(cq.Workplane("XY").add(shape), os.path.join(out, name + ".step"))
    cq.exporters.export(cq.Workplane("XY").add(shape), os.path.join(out, name + ".stl"),
                        tolerance=0.05, angularTolerance=0.2)


if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
    os.makedirs(out, exist_ok=True)
    base, mezz, top, th, sh = body_base(), mezzanine(), top_plate(), thigh(), shin()
    for n, s in [("body_base", base), ("mezzanine", mezz), ("top_plate", top),
                 ("thigh", th), ("shin", sh), ("mg995_reference", mg995())]:
        v = s.val()
        bb = v.BoundingBox()
        print(f"{n:16s} valid={v.isValid()} vol={v.Volume()/1000:8.1f} cm3 "
              f"bbox={bb.xlen:.0f}x{bb.ylen:.0f}x{bb.zlen:.0f}")
        export(n, s, out)
    asm = cq.Compound.makeCompound(
        [base.val(), mezz.val(), top.val()] + assemble(0, 0))
    export("assembly_standing", asm, out)
    worst, bad = clearance_report(base)
    print("clearance: worst overlap with body = %.1f mm3 over %d bad poses" % (worst, len(bad)))
    for b in bad[:10]:
        print("  theta=%d phi=%d overlap=%s mm3" % b)
