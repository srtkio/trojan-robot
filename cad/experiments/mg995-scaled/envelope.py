"""Largest symmetric hip swing (deg) at which front and rear legs on one side never touch.
Run: python envelope.py"""
import itertools
from functools import lru_cache

import sesame_mg995 as m


def make_free(phis=(-45, 0, 45)):
    @lru_cache(maxsize=None)
    def pose(theta, phi, end):
        return m.place_leg(m.leg_canonical(theta, phi, rear=end < 0), 1, end)

    def free(A):
        for tf, tr in itertools.product((-A, 0, A), repeat=2):
            for pf, pr in itertools.product(phis, repeat=2):
                a, b = pose(tf, pf, 1), pose(tr, pr, -1)
                if any(x.intersect(y).Volume() > 1 for x in a for y in b):
                    return False
        return True
    return free


for l1, l2 in ((100, 120), (80, 100), (70, 90), (65, 80)):
    m.L1, m.L2 = l1, l2
    free = make_free()
    best = 0
    for A in range(10, 61, 5):
        if not free(A):
            break
        best = A
    print(f"L1={l1} L2={l2}: collision-free hip swing +/-{best} deg, hanging height {l1 + l2} mm",
          flush=True)
