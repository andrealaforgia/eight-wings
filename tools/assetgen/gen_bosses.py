"""Three hi-res modular bosses, with sockets in logical body coordinates.

Gun centres are (20,36)/(76,36); core centre is (48,42). At SCALE=4
these are 64-texel recesses, matching the separately rendered attachments.
"""
from pixlib import emit
from _jobb_hires import Drawing, STEEL, RED, GOLD, SAND, TEAL, PURPLE

RAMPS = {1: STEEL, 2: TEAL, 3: PURPLE}
ACCENTS = {1: RED, 2: TEAL, 3: RED}


def sockets(d, boss):
    ramp = RAMPS[boss]
    for x, y in ((20, 36), (76, 36), (48, 42)):
        d.panel(x - 9.5, y - 9.5, 19, 19, ramp)
        d.rect(x - 8, y - 8, 16, 16, 'ink')
        d.rect(x - 7, y - 7, 14, 14, ramp[1])
        d.rect(x - 5, y - 5, 10, 10, 'ink')
        # A concentric bearing and a few connectors remain visible after destruction.
        d.oval(x, y, 4, 4, (ramp[0], ramp[0], ramp[1], ramp[1], ramp[2], ramp[2]))
        d.rect(x - 2, y - 2, 4, 4, ramp[1])
        for xx, yy in ((x - 8.5, y - 8.5), (x + 8, y - 8.5),
                       (x - 8.5, y + 8), (x + 8, y + 8)):
            d.bolt(xx, yy, ramp[4])
        for xx in (x - 4, x, x + 4):
            d.rect(xx, y + 6.5, 1, .5, ramp[2])
        d.line(x - 6, y - 7.5, x + 6, y - 7.5, ramp[2])


def paired_panel(d, x, y, w, h, ramp=STEEL):
    d.panel(x, y, w, h, ramp)
    d.panel(96 - x - w, y, w, h, ramp)


def armour_details(d, boss):
    """Etched seams follow each hull's structure, clipped to existing armour."""
    detail = Drawing(96, 56)
    ramp = RAMPS[boss]
    for side in (-1, 1):
        def x(v): return v if side < 0 else 95.5 - v
        shine = ramp[4] if side < 0 else ramp[3]
        if boss == 1:
            for points in (
                [(22, 24), (25, 26), (32, 27)],
                [(25, 10), (31, 6), (38, 4.5)],
                [(22, 25), (22, 29), (25, 31)],
                [(30, 43), (32, 46), (37, 46)],
                [(38, 9), (40, 11), (40, 24)],
            ):
                for a, b in zip(points, points[1:]):
                    detail.line(x(a[0]), a[1], x(b[0]), b[1], ramp[1])
                    detail.line(x(a[0]), a[1] - .5, x(b[0]), b[1] - .5, shine)
            for xx, yy in ((25, 17), (25, 21), (32, 8), (39, 27), (31, 46)):
                detail.bolt(x(xx), yy, shine)
            for yy in (12, 14, 16, 18, 20):
                detail.line(x(21), yy, x(22), yy + 1, 'g1')
            for yy in (25, 26.5):
                detail.line(x(28), yy, x(36), yy, 'g1')
                detail.line(x(28), yy - .5, x(34), yy - .5, 'g4')
            # Narrow conduits link the generator to the gun bearings.
            detail.pipe([(x(39), 28), (x(38), 32), (x(38), 39)], STEEL, 3)
            for yy in (30, 34, 38):
                detail.line(x(37), yy, x(39), yy, 'g1')
            for xx in (4, 8):
                detail.line(x(xx), 9, x(xx + 3), 9, 'r1')
                detail.line(x(xx), 37, x(xx + 3), 37, 'r1')
        elif boss == 2:
            for xx, yy in ((8, 25), (18, 17), (29, 11)):
                for offset in (0, 2, 4):
                    detail.line(x(xx + 2), yy + 10 + offset,
                                x(xx + 5), yy + 12 + offset, 'c1')
                    detail.line(x(xx + 5), yy + 12 + offset,
                                x(xx + 10), yy + 11 + offset, 'b1')
                    detail.line(x(xx + 2), yy + 9.5 + offset,
                                x(xx + 5), yy + 11.5 + offset, shine)
                detail.line(x(xx + 11), yy + 2, x(xx + 12), yy + 8, 'b1')
                detail.line(x(xx + 11.5), yy + 2, x(xx + 12.5), yy + 8, 'c3')
                for zz in (0, 1.5, 3):
                    detail.point(x(xx + 1), yy + 6 + zz, 'c4')
                detail.bolt(x(xx + 10), yy + 9, shine)
            detail.pipe([(x(35), 34), (x(38), 36), (x(38), 43)], TEAL, 3)
            for yy in (35, 38, 41):
                detail.line(x(35), yy, x(36), yy + 1.5, 'c4')
            for xx in (32, 35):
                detail.line(x(xx), 23, x(xx + 1.5), 25, 'b1')
        else:
            # Parallel armour feathers with inset copper-red energy traces.
            for xx, yy in ((8, 10), (14, 13), (20, 16), (26, 19)):
                detail.line(x(xx), yy, x(xx + 4), yy + 3, 'p1')
                detail.line(x(xx), yy + .5, x(xx + 3.5), yy + 3, shine)
                detail.line(x(xx + 1), yy + 1.5, x(xx + 4), yy + 4, 'm1')
                detail.point(x(xx + 1), yy + 1.5, 'r4')
                detail.point(x(xx + 3), yy + 3, 'p1')
            for points in (
                [(13, 10), (26, 15), (35, 23)],
                [(8, 16), (13, 23), (25, 28)],
                [(22, 44), (29, 41), (33, 37)],
                [(32, 29), (36, 26), (38, 21)],
            ):
                for a, b in zip(points, points[1:]):
                    detail.line(x(a[0]), a[1], x(b[0]), b[1], 'p1')
                    detail.line(x(a[0]), a[1] - .5, x(b[0]), b[1] - .5, shine)
            for xx, yy in ((11, 19), (16, 24), (23, 27), (33, 25), (37, 33), (29, 43)):
                detail.bolt(x(xx), yy, shine)
            detail.pipe([(x(37), 35), (x(39), 38), (x(39), 44)], PURPLE, 3)
            for yy in (23, 26, 29):
                detail.line(x(37), yy, x(38), yy - 1, 'r4')
    for y in range(d.c.h):
        for x in range(d.c.w):
            if d.c.get(x, y)[3] and detail.c.get(x, y)[3]:
                d.c.set(x, y, detail.c.get(x, y))


def machinery(d, boss):
    """Structural subassemblies, with final sockets drawn over their cable runs."""
    detail = Drawing(96, 56)
    ramp = RAMPS[boss]
    for side in (-1, 1):
        def x(v): return v if side < 0 else 95.75 - v
        if boss == 1:
            for yy in (5, 33):
                for xx in (3.8, 8.8):
                    detail.panel(x(xx) - 1.3, yy + 3.3, 2.6, 2.6, RED)
                    detail.line(x(xx) - .7, yy + 4, x(xx) + .7, yy + 4, 'r4')
                for xx in (4, 9, 14):
                    for y in (yy + 10, yy + 12, yy + 14):
                        detail.line(x(xx), y, x(xx + 1), y, 'g4')
                        detail.line(x(xx), y + .25, x(xx + 1), y + .25, 'g1')
            # Curved shoulders, inset numbered service panels and hydraulic manifolds.
            detail.seam([(x(25), 11), (x(31), 7), (x(38), 5.5)], ramp)
            detail.panel(x(25.5) - 1, 13, 2, 10, STEEL)
            for y in (14.5, 17, 19.5):
                detail.oval(x(25.5), y, .5, .6, RED)
                detail.point(x(25.5) - .2, y - .2, 'r4')
            for xx in (28, 30, 32, 34, 36):
                detail.line(x(xx), 9, x(xx + .6), 9.6, 'g1')
                detail.point(x(xx), 8.75, 'g4')
            for yy in (25, 27):
                detail.panel(x(32) - 3, yy, 6, 1.2, STEEL)
                detail.line(x(30), yy + .5, x(33), yy + .5, 'g1')
            detail.panel(x(37), 31, 3, 10, STEEL)
            detail.vent(x(37.5), 32, 2, 6, 'g2')
            detail.pipe([(x(24), 27), (x(29), 31), (x(30), 36)], STEEL, 6)
            for i in range(5):
                detail.line(x(24.5 + i), 27.6 + i * .8,
                            x(25 + i), 27.1 + i * .8, 'g1')
            detail.panel(x(32) - 2, 42, 4, 3, RED)
            detail.rect(x(32) - 1.2, 42.8, 2.2, 1, 'r1')
            detail.line(x(32) - 1.2, 42.8, x(32) + .3, 42.8, 'o3')
        elif boss == 2:
            for xx, yy in ((8, 25), (18, 17), (29, 11)):
                # Pressure hull collars curve around each vertebral segment.
                for band in (11, 14, 17):
                    detail.curve([(x(xx + .5), yy + band - 1),
                                  (x(xx + 3), yy + band + 3),
                                  (x(xx + 10), yy + band + 3),
                                  (x(xx + 13), yy + band - 1)], 'b1', 2)
                    detail.curve([(x(xx + .5), yy + band - 1.25),
                                  (x(xx + 3), yy + band + 2.75),
                                  (x(xx + 10), yy + band + 2.75),
                                  (x(xx + 13), yy + band - 1.25)], 'c3')
                for i in range(4):
                    detail.panel(x(xx + 12) - .5, yy + 2 + i * 1.4, 1, .8, TEAL)
                    detail.point(x(xx + 12) - .25, yy + 2 + i * 1.4, 'c4')
                detail.bolt(x(xx + 2), yy + 8, 'c4')
                detail.bolt(x(xx + 11), yy + 8, 'c4')
                detail.curve([(x(xx + 3), yy + 3), (x(xx + 3), yy - 1),
                              (x(xx + 8), yy - 2), (x(xx + 10), yy + 1)], 'c4')
            for xx, yy in ((3, 25), (11, 14), (23, 6), (36, 2)):
                detail.curve([(x(xx), yy), (x(xx + 4), yy + 2),
                              (x(xx + 6), yy + 5), (x(xx + 10), yy + 11)], 'c2')
                detail.line(x(xx + 3), yy + 3, x(xx + 7), yy + 7, 'c4')
            detail.pipe([(x(37), 31), (x(39), 33), (x(39), 42)], STEEL, 5)
            for yy in (34, 36, 38, 40):
                detail.line(x(38.5), yy, x(40), yy, 'c1')
        else:
            # Overlapping sabre feathers with a fine inlaid copper/magenta circuit.
            for i in range(5):
                xx, yy = 5 + i * 4.9, 8.5 + i * 2.8
                detail.plate([(x(xx), yy), (x(xx + 5.5), yy + 1.4),
                              (x(xx + 9), yy + 6), (x(xx + 6), yy + 7.5),
                              (x(xx + 3), yy + 4)], PURPLE)
                detail.curve([(x(xx + .8), yy + 1), (x(xx + 3), yy + 2),
                              (x(xx + 4), yy + 5), (x(xx + 7), yy + 6.3)], 'm1', 2)
                detail.line(x(xx + 2), yy + 2.2, x(xx + 4), yy + 3.5, 'r4')
                for k in range(3):
                    detail.point(x(xx + 4 + k * .8), yy + 2.3 + k * .6, 'p1')
                detail.bolt(x(xx + 5.4), yy + 5.4, 'p4')
            detail.seam([(x(18), 39), (x(29), 36), (x(34), 32)], PURPLE)
            detail.curve([(x(20), 44), (x(27), 43), (x(32), 39), (x(35), 36)], 'm2')
            for xx in (39, 43):
                detail.seam([(x(xx), 7), (x(xx + 1), 11), (x(xx + 1), 13)], RED)
            for yy in (23.5, 25.5, 27.5, 29.5):
                detail.line(x(36.5), yy, x(38), yy, 'm2')
                detail.point(x(36.5), yy, 'r4')
        for xx, yy in ((26, 26), (34, 44), (39, 10)):
            detail.bolt(x(xx), yy, ramp[4])
    # Clip to hull material, preserving the designed negative spaces and outline.
    for y in range(d.c.h):
        for x in range(d.c.w):
            if d.c.get(x, y)[3] and detail.c.get(x, y)[3] and d.c.get(x, y) != d.c._rgba('ink'):
                d.c.set(x, y, detail.c.get(x, y))


def walker():
    d = Drawing(96, 56)
    # Four splayed hydraulic feet; the negative spaces preserve their articulation.
    for y in (5, 33):
        d.plate([(2, y + 2), (11, y), (17, y + 3), (19, y + 8),
                 (16, y + 17), (3, y + 17), (1, y + 12)], STEEL, True)
        paired_panel(d, 3, y + 3, 10, 5, RED)
        for x in (3, 8, 13):
            paired_panel(d, x, y + 9, 2.5, 7, STEEL)
            for side in (-1, 1):
                xx = x + 1 if side < 0 else 95 - x - 1
                d.line(xx, y + 10, xx, y + 14, 'g1')
        for x in (4, 11):
            d.bolt(x, y + 4)
            d.bolt(95 - x, y + 4)
    paired_panel(d, 14, 17, 8, 22)
    for x in (15.5, 78.5):
        d.pipe([(x, 18), (x, 37)], STEEL, 5)
        for y in range(20, 36, 3):
            d.line(x - .5, y, x + 2.5, y, 'g1')
            d.line(x - .5, y + .5, x + 2.5, y + .5, 'g4')
    d.plate([(24, 11), (32, 5), (41, 3), (48, 6), (48, 50),
             (34, 50), (20, 43), (19, 25)], STEEL, True)
    d.plate([(27, 13), (35, 9), (48, 11), (48, 30), (32, 29),
             (23, 23)], STEEL, True)
    paired_panel(d, 27, 12, 12, 12, STEEL)
    for x in (28, 57):
        d.panel(x, 13, 10, 4, RED)
        d.rect(x + 1, 14, 8, 1.5, 'r1')
        d.line(x + 1, 14, x + 7, 14, 'o3')
        d.point(x + 1, 14, 'y2')
        d.vent(x + 1, 19, 8, 3, 'r2')
        d.bolt(x, 12)
        d.bolt(x + 8, 23)
    paired_panel(d, 41, 12, 6, 15)
    for x in (42, 50):
        d.vent(x, 14, 4, 10)
    d.panel(41, 4, 14, 5, RED)
    d.rect(43, 5, 10, 2, 'o2')
    d.line(43, 5, 51, 5, 'y1')
    d.point(43, 5, 'white')
    d.line(48, 4, 48, 9, 'g1')
    for x in (23, 70):
        d.plate([(x, 11), (x + 2, 7), (x + 3, 19), (x, 24)], STEEL)
        d.line(x + .5, 12, x + .5, 19, 'g4')
    for x in (32, 59):
        d.vent(x, 30, 5, 10, 'o2')
        d.pipe([(x + 1, 42), (x + 1, 45), (x + 5, 47)], STEEL, 4)
    for x, y in ((23, 26), (31, 9), (39, 25), (30, 45), (43, 10)):
        d.bolt(x, y)
        d.bolt(95 - x, y)
    for x in range(33, 42):
        d.line(x, 48, x - .5, 49, 'y1' if x % 2 else 'ink')
        d.line(95 - x, 48, 95.5 - x, 49, 'y1' if x % 2 else 'ink')
    machinery(d, 1)
    armour_details(d, 1)
    sockets(d, 1)
    return d.finish()


def hydra():
    d = Drawing(96, 56)
    # Swept-back fins and overlapping pressure segments.
    for x, y in ((2, 24), (10, 13), (22, 5), (35, 1)):
        d.plate([(x, y), (x + 9, y + 4), (x + 14, y + 16),
                 (x + 8, y + 13), (x + 6, y + 7)], TEAL, True)
        for side in (-1, 1):
            def xx(v): return v if side < 0 else 95.5 - v
            d.line(xx(x + 2), y + 2, xx(x + 9), y + 12, 'c4' if side < 0 else 'c2')
            d.line(xx(x + 4), y + 5, xx(x + 9), y + 13, 'b1')
    for x, y in ((8, 25), (18, 17), (29, 11), (40, 9)):
        d.plate([(x, y), (x + 7, y - 3), (x + 14, y + 1), (x + 15, y + 15),
                 (x + 9, y + 23), (x, y + 18)], TEAL, True)
        for side in (-1, 1):
            def xx(v): return v if side < 0 else 95.5 - v
            for yy in (10, 13, 16):
                d.line(xx(x + 1), y + yy, xx(x + 9), y + yy + 3, 'b1')
                d.line(xx(x + 1), y + yy - .5, xx(x + 9), y + yy + 2.5,
                       'c3' if side < 0 else 'c2')
            cx = xx(x + 7)
            d.glass(cx, y + 4, 4, 4.5, TEAL)
            d.line(cx - 1.5, y + 1, cx - 1.5, y + 6, 'c1')
            d.line(cx + 1, y + 1, cx + 1, y + 6, 'c1')
            d.bolt(xx(x + 1), y + 3, 'c4')
    d.plate([(35, 28), (43, 23), (48, 24), (48, 53), (41, 51),
             (32, 40)], TEAL, True)
    paired_panel(d, 34, 27, 8, 4, TEAL)
    for x in (35, 55):
        d.rect(x, 28, 6, 1, 'r1')
        d.line(x, 28, x + 4, 28, 'r4')
        d.point(x, 28, 'y2')
    paired_panel(d, 43, 12, 4.5, 16, TEAL)
    for x in (44, 49):
        d.vent(x, 15, 3, 10, 'c2')
    # Split jaw prongs remain visible below the reactor mounting.
    d.plate([(31, 40), (36, 43), (39, 53), (34, 49)], TEAL, True)
    for x in (34, 59):
        d.line(x, 44, x + 2, 49, 'c4')
        d.point(x + 2, 50, 'white')
    machinery(d, 2)
    armour_details(d, 2)
    sockets(d, 2)
    return d.finish()


def queen():
    d = Drawing(96, 56)
    # Pierced sabre wings and secondary scythes frame the central crown.
    d.plate([(2, 5), (15, 8), (29, 15), (40, 24), (34, 31),
             (17, 26), (7, 17)], PURPLE, True)
    d.plate([(6, 30), (20, 30), (35, 28), (43, 35), (35, 44),
             (16, 52), (23, 40)], PURPLE, True)
    for side in (-1, 1):
        def xx(v): return v if side < 0 else 95.5 - v
        d.flat([(xx(8), 10), (xx(17), 13), (xx(32), 24), (xx(18), 20)], 'm2' if side < 0 else 'm1')
        d.line(xx(5), 8, xx(33), 25, 'p4' if side < 0 else 'p3')
        d.line(xx(10), 15, xx(29), 25, 'p1')
        d.flat([(xx(11), 17), (xx(22), 23), (xx(16), 22)], 'ink')
        d.flat([(xx(14), 32), (xx(25), 34), (xx(33), 32), (xx(27), 37), (xx(18), 35)], 'p1')
        d.line(xx(15), 32.5, xx(25), 35, 'r3')
        d.line(xx(24), 42, xx(34), 37, 'r4' if side < 0 else 'r3')
        for x, y in ((12, 11), (19, 15), (26, 19), (33, 23)):
            d.line(xx(x), y, xx(x + 1), y + 4, 'p1')
            d.line(xx(x - .5), y, xx(x + .5), y + 3, 'p4' if side < 0 else 'p3')
            d.bolt(xx(x), y, 'p4')
        d.plate([(xx(3), 25), (xx(15), 27), (xx(10), 29)], RED)
    d.plate([(39, 13), (43, 18), (48, 13), (48, 50), (41, 48),
             (35, 37), (35, 23)], PURPLE, True)
    d.plate([(38, 14), (39, 2), (44, 10), (48, 1), (48, 21), (41, 21)], RED, True)
    for x in (40, 44, 48, 51, 55):
        d.line(x, 8 if x != 48 else 4, x, 13, 'r4' if x < 48 else 'r3')
    d.panel(41, 15, 14, 5, PURPLE)
    d.rect(42, 16, 12, 2.5, 'r1')
    d.line(43, 16.5, 51.5, 16.5, 'm2')
    d.line(43, 16, 50, 16, 'p4')
    d.point(43, 16, 'white')
    d.panel(40, 22, 16, 10, PURPLE)
    for x in (42, 45, 49, 52):
        d.rect(x, 24, 1, 6, 'r1')
        d.line(x, 24, x, 29, 'r4' if x < 48 else 'm2')
    d.plate([(35, 42), (40, 46), (41, 53), (36, 50)], RED, True)
    for x in (37, 56):
        d.line(x, 44, x + 2, 49, 'r4')
    machinery(d, 3)
    armour_details(d, 3)
    sockets(d, 3)
    return d.finish()


def core(boss, f):
    d = Drawing(16, 16)
    ramp = RAMPS[boss]
    d.panel(1, 1, 13, 13, ramp)
    d.panel(3, 2.5, 9, 10.5, ramp)
    if boss == 1:
        d.rect(4, 3.5, 7, 8.5, 'r1')
        d.glass(7.5, 7.5, 3, 4, ('r1', 'r2', 'o2', 'o3', 'y2', 'white'))
        for y in (4.5, 7.5, 10.5):
            d.line(4, y, 10.5, y, 'r2' if not f else 'y2')
        if f:
            d.rect(6, 5, 3, 5, 'white')
        d.line(5.5, 5, 5.5, 9, 'y2')
    elif boss == 2:
        d.flat([(7.5, 3), (11, 7), (7.5, 12), (4, 7)], 'c3')
        d.flat([(7.5, 3.5), (7.5, 10.5), (4.5, 7)], 'c4')
        d.flat([(7.5, 4), (10, 7), (7.5, 11), (6, 7)], 'white' if f else 'c2')
        d.line(7.5, 4, 7.5, 8, 'white')
    else:
        d.oval(7.5, 7.5, 5, 3.7, RED)
        d.oval(7.5, 7.5, 3.5, 2.7, ('r1', 'm1', 'm2', 'p3', 'p4', 'white'))
        if f:
            d.oval(7.5, 7.5, 2.5, 2, ('p4', 'white', 'white', 'white', 'white', 'white'))
        d.rect(7, 5.5, 1, 4, 'p1' if not f else 'y2')
        d.point(6, 6, 'white')
    for x in (1.5, 12.5):
        for y in (2, 5, 8, 11):
            d.line(x, y, x + 1, y, ramp[4])
    for x, y in ((2, 2), (12, 2), (2, 12), (12, 12)):
        d.bolt(x, y, ramp[4])
    return d.finish()


def gun(boss, f):
    d = Drawing(16, 16)
    ramp = RAMPS[boss]
    recoil = 1.5 if f == 3 else 0
    d.oval(7.75, 7, 6.5, 6, ramp)
    if boss == 1:
        d.panel(3, 3, 9, 6, RED)
        d.vent(4, 4, 7, 3, 'r2')
        for x in (4, 9):
            d.panel(x, 7 - recoil, 2.5, 7, ramp)
            for y in (8, 10, 12):
                d.line(x, y - recoil, x + 2, y - recoil, 'g1')
            d.rect(x + .5, 12.5 - recoil, 1.5, 1, ('ink', 'o3', 'white', 'r1')[f])
    elif boss == 2:
        d.plate([(7.75, 1), (13, 4), (12, 10), (7.75, 14), (3, 10), (2, 4)], ramp)
        d.panel(5.5, 4, 4.5, 9 - recoil, ramp)
        d.line(6.5, 5, 6.5, 10 - recoil, 'c4')
        d.rect(6, 11 - recoil, 3.5, 1, ('b1', 'c3', 'white', 'c1')[f])
        for y in (6, 8, 10):
            d.line(3, y, 4.5, y + .5, 'b1')
            d.line(11, y, 12.5, y + .5, 'b1')
    else:
        d.plate([(3, 3), (7.75, 5), (12.5, 3), (14, 7), (11, 12),
                 (7.75, 14), (4, 12), (1.5, 7)], ramp)
        for side in (-1, 1):
            x = 7.75 + side * 4
            d.plate([(x, 5), (x - side * 2.5, 7), (x - side, 12 - recoil)], RED)
        d.panel(6, 6 - recoil, 3.5, 7, ramp)
        d.line(7, 7 - recoil, 7, 11 - recoil, 'm2')
        d.rect(6.5, 12 - recoil, 2.5, 1, ('r1', 'm2', 'white', 'p1')[f])
    for side in (-1, 1):
        x = 7.75 + side * 4.5
        d.bolt(x, 7.5, ramp[4])
        d.line(x - .4, 8.5, x + .4, 8.5, ramp[1])
        d.line(x - .4, 8.25, x + .4, 8.25, ramp[4])
    d.curve([(3, 5), (3.5, 1.8), (8, .8), (11, 3)], ramp[4])
    d.bolt(3, 4, ramp[4])
    d.bolt(11.5, 4, ramp[4])
    if f == 1:
        d.oval(7.75, 7, 1.3, 1.3, GOLD if boss == 1 else ramp)
        d.point(7.25, 6.5, 'white')
    if f == 2:
        d.flat([(5.5, 13), (6.5, 14), (7.75, 15.5), (9, 14), (10, 13)], 'o3' if boss == 1 else 'm2')
        d.line(6.5, 13.5, 9, 13.5, 'y2')
        d.line(7.75, 13, 7.75, 15, 'white')
    return d.finish()


def generate():
    for boss, draw in ((1, walker), (2, hydra), (3, queen)):
        emit('boss%d_body' % boss, draw(), group='bosses')
        for f in range(2):
            emit('boss%d_core' % boss, core(boss, f), frame=f, group='bosses')
        for f in range(4):
            emit('boss%d_gun' % boss, gun(boss, f), frame=f, group='bosses')


if __name__ == '__main__':
    generate()
