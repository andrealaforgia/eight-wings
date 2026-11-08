"""Shared top-down anatomical landmarks for theropods and their skeletons."""
import math
from _jobb_hires import Drawing, SCALE, GREEN, BONE

REX = ('ink', 'r1', 's2', 'o2', 's3', 's4')


def limb(d, a, b, width, ramp):
    dx, dy = b[0] - a[0], b[1] - a[1]
    length = math.hypot(dx, dy)
    nx, ny = -dy / length * width, dx / length * width
    d.plate([(a[0] + nx, a[1] + ny), (b[0] + nx * .5, b[1] + ny * .5),
             (b[0] - nx * .5, b[1] - ny * .5), (a[0] - nx, a[1] - ny)], ramp)
    d.line(a[0] - .2, a[1] - .25, b[0] - .2, b[1] - .25, ramp[4])


def bone(d, a, b, width=.45):
    limb(d, a, b, width, BONE)
    for x, y in (a, b):
        d.oval(x, y, width * 1.1, width * .85, BONE)


def dinosaur(size, frame=0, bones=False):
    large = size == 32
    d = Drawing(size, size)
    cx = size / 2 - .125
    hip, shoulder, nose = (14.5, 21, 29) if large else (9, 12.7, 18)
    body, skull = (5.1, 5.4) if large else (3.1, 3.1)
    swing = (0, .85, 0, -.85)[frame]
    stride = (-.5, 1.4, .5, -1.4)[frame]
    tip = (cx + 2 + swing, 1.3)
    legs = []
    for side in (-1, 1):
        step = stride * side
        legs.append(((cx + side * body * .55, hip),
                     (cx + side * (body + 2.4), hip + .8 + step),
                     (cx + side * (body + 1.6), hip + 4.9 + step)))
    if bones:
        # Neutral landmarks are shared exactly with live frame zero.
        d.curve([(cx, hip), (cx - 2.4, hip - 5), (cx - .6, 5), tip], 's2', 3)
        d.curve([(cx - .2, hip), (cx - 2.6, hip - 5), (cx - .8, 5), tip], 's5', 2)
        for y in range(4, int(hip - 2)):
            t = (hip - y) / (hip - 1.3)
            x = cx - math.sin(t * math.pi) * 1.3 + t * 2
            d.line(x - .35, y, x + .35, y + .12, 'white')
        bone(d, (cx, hip - 3), (cx, shoulder + .6), .48)
        for y in range(int(hip - 2), int(shoulder), 1 if large else 2):
            d.oval(cx, y, .7, .35, BONE)
            reach = body * (.85 - abs(y - hip - 1.5) / 12)
            for side in (-1, 1):
                points = [(cx + side * .55, y), (cx + side * reach, y - .45),
                          (cx + side * (reach + .6), y + 1.2), (cx + side * 1.1, y + 1.65)]
                d.curve(points, 's2', 3)
                points = [(x - .15, yy - .15) for x, yy in points]
                d.curve(points, 's5', 2)
                d.point(cx + side * reach, y + .2, 'white')
        for root, knee, foot in legs:
            bone(d, root, knee, .55 if large else .4)
            bone(d, knee, foot, .4 if large else .3)
            d.line(knee[0] + .5, knee[1], foot[0] + .5, foot[1] - .5, 'g4')
            for toe in (-1, 0, 1):
                bone(d, foot, (foot[0] + toe * 1.1, foot[1] + 1.8), .23)
        for side in (-1, 1):
            bone(d, (cx + side * 1.4, shoulder - 1),
                 (cx + side * (body + .7), shoulder + .8), .35)
        d.oval(cx, nose - 3.5, skull, 3.8 if large else 2.6, BONE)
        # Separate cheekbones, nasal opening, temporal fenestrae and teeth.
        for side in (-1, 1):
            d.oval(cx + side * skull * .58, nose - 4.3, skull * .27, 1.1 if large else .7,
                   ('s2', 's2', 'g1', 'g1', 'ink', 'ink'))
            d.oval(cx + side * skull * .45, nose - 6 if large else nose - 5,
                   .7 if large else .4, .6 if large else .4,
                   ('s2', 's2', 'g1', 'g1', 'ink', 'ink'))
            d.curve([(cx + side * (skull - .5), nose - 3),
                     (cx + side * skull, nose), (cx + side * 2.3, nose + .3),
                     (cx + side * .7, nose + .3)], 's5', 2)
        d.flat([(cx, nose - 3), (cx - .7, nose - 1.7), (cx + .7, nose - 1.7)], 'g1')
        d.rect(cx - skull + 1.1, nose - .7, skull * 2 - 2.2, .5, 'g1')
        for i in range(8 if large else 5):
            x = cx - skull + 1.3 + i * (skull * 2 - 2.6) / (7 if large else 4)
            d.line(x, nose - .8, x, nose + .1, 'white')
        d.seam([(cx - 1, nose - 6), (cx - .3, nose - 4), (cx - .7, nose - 3)], BONE)
        return d.finish(True)

    ramp = REX if large else GREEN
    for side, (root, knee, foot) in zip((-1, 1), legs):
        limb(d, root, knee, 1.8 if large else 1.1, ramp)
        d.oval(knee[0], knee[1], 1.8 if large else 1.15, 2.2 if large else 1.4, ramp)
        limb(d, knee, foot, .95 if large else .6, ramp)
        for toe in (-1, 0, 1):
            end = (foot[0] + toe * 1.1, foot[1] + 1.8)
            limb(d, foot, end, .3, ramp)
            d.line(end[0], end[1] - .5, end[0] + toe * .2, end[1] + .2, 's5', 2)
            d.point(end[0] - .25, end[1], 'white')
        for band in range(3):
            y = knee[1] + 1 + band * .65
            d.line(knee[0] - .4, y, knee[0] + .35, y + .2, ramp[1])
    # Curved, tapering tail replaces the former straight triangular wedge.
    points = []
    for i in range(33):
        t, u = i / 32, 1 - i / 32
        x = u**3*cx + 3*u*u*t*(cx-3) + 3*u*t*t*(cx-.5) + t**3*tip[0]
        y = u**3*hip + 3*u*u*t*(hip-5) + 3*u*t*t*5 + t**3*tip[1]
        points.append((x, y, 2 * u**1.6 + .12))
    d.plate([(x - w, y) for x, y, w in points] + [(x + w, y) for x, y, w in reversed(points)], ramp)
    d.curve([(cx - .7, hip), (cx - 3, hip - 5), (cx - .9, 5), tip], ramp[4])
    d.oval(cx, hip + 1.1, body, 6 if large else 3.9, ramp)
    # Flank scale rows follow the lit volume; their sparse highlights form coherent bands.
    for side in (-1, 1):
        for row in range(6 if large else 4):
            y = hip - 1.8 + row * .85
            reach = body * math.sqrt(max(.1, 1 - ((y - hip - 1.1) / (6 if large else 3.9))**2))
            for col in range(2 if large else 1):
                x = cx + side * (reach - .7 - col * .85)
                d.curve([(x - .35, y), (x - .3, y + .4), (x + .2, y + .5), (x + .4, y + .1)], ramp[1])
                d.point(x - .25, y - .15, ramp[4] if side < 0 else ramp[3])
        arm = [(cx + side * 1.4, shoulder - 1),
               (cx + side * (body + .7), shoulder + .2),
               (cx + side * (body + .1), shoulder + 1.2)]
        limb(d, arm[0], arm[1], .65, ramp)
        limb(d, arm[1], arm[2], .4, ramp)
        for toe in (0, .5):
            d.line(arm[2][0] + toe, arm[2][1], arm[2][0] + toe - .3, arm[2][1] + .5, 's5')
    d.oval(cx, shoulder, 2.6 if large else 1.7, 3.6 if large else 2.3, ramp)
    d.oval(cx, nose - 3.6, skull, 4.4 if large else 3, ramp)
    d.plate([(cx - skull + .8, nose - 2.6), (cx + skull - .8, nose - 2.6),
             (cx + skull - 1.3, nose + .3), (cx - skull + 1.3, nose + .3)], ramp)
    d.curve([(cx - skull + .7, nose - 1), (cx - 2, nose - .2),
             (cx + 2, nose - .2), (cx + skull - .7, nose - 1)], ramp[1], 2)
    for side in (-1, 1):
        ex = cx + side * (skull - 1.1)
        d.oval(ex, nose - 4.6, .85 if large else .55, .7 if large else .5,
               ('ink', 'r1', 'r2', 'o3', 'y1', 'y2'))
        d.line(ex, nose - 5, ex, nose - 4.2, 'ink', 2)
        d.point(ex - .3, nose - 4.9, 'white')
        d.curve([(ex - .8, nose - 4.6), (ex - .5, nose - 5.7),
                 (ex + .7, nose - 5.8), (ex + 1, nose - 4.8)], 'r3' if large else 'n4', 2)
        d.oval(cx + side * 1.2, nose - 1.8, .3, .4, (ramp[1],)*6)
        for i in range(3 if large else 2):
            d.point(cx + side * (1.4 + i * .65), nose - .4, 's5')
        for row in range(3):
            x, y = cx + side * (skull - 1.5), nose - 3.5 + row * .6
            d.line(x - .3, y, x + .2, y + .15, ramp[2])
        d.point(cx + side * (skull - .6), nose - 2.3, 'r3' if large else 'o2')
    for y in range(int(hip - 5), int(shoulder - 1)):
        crest = .6 if large else .45
        d.plate([(cx - crest, y + .6), (cx - .15, y - .4), (cx + crest, y + .7)],
                ('ink', 'r1', 'r2', 'r3', 'o3', 'y1'))
    d.line(cx - 1, nose - 6.2, cx + .5, nose - 6.2, ramp[4])
    return d.finish(True)
