"""Interference checks for the Sesame-MG995 model. Run: python check_clearance.py"""
import sesame_mg995 as m


def ov(a, b):
    return a.intersect(b).Volume()


base = m.body_base().val()

print("-- sanity: the swing check must detect a collision when one exists")
for th in (60, 90, 120, 150):
    parts = m.place_leg(m.leg_canonical(th, 0), 1, 1)
    print(" theta", th, "overlap with body [thigh, servo, shin]:",
          [round(ov(base, p), 1) for p in parts])

print("-- neutral pose, front-left leg")
th, sv, sh = m.place_leg(m.leg_canonical(0, 0), 1, 1)
hip = m.at(m.mg995(), *m.hip_plane(1, 1))
print(" hip servo vs body:", round(ov(base, hip), 1))
print(" hip servo vs thigh:", round(ov(hip, th), 1))
print(" knee servo vs thigh:", round(ov(sv, th), 1))
print(" knee servo vs shin:", round(ov(sv, sh), 1), " thigh vs shin:", round(ov(th, sh), 1))
for n, p in (("thigh", th), ("knee servo", sv), ("shin", sh), ("hip servo", hip)):
    b = p.BoundingBox()
    print(" %-10s x[%.1f,%.1f] y[%.1f,%.1f] z[%.1f,%.1f]" %
          (n, b.xmin, b.xmax, b.ymin, b.ymax, b.zmin, b.zmax))

print("-- swing range vs body (theta -60..60, phi -60..60)")
worst, bad = m.clearance_report(m.body_base())
print(" worst overlap %.1f mm3, %d bad poses" % (worst, len(bad)))

print("-- front vs rear leg, same side")
hits = []
for tf in range(-60, 61, 15):
    for tr in range(-60, 61, 15):
        for pf in (-60, 0, 60):
            for pr in (-60, 0, 60):
                a = m.place_leg(m.leg_canonical(tf, pf), 1, 1)
                b = m.place_leg(m.leg_canonical(tr, pr, rear=True), 1, -1)
                v = max(ov(x, y) for x in a for y in b)
                if v > 1:
                    hits.append((tf, pf, tr, pr, round(v, 1)))
print(" colliding combos:", len(hits), hits[:5])
