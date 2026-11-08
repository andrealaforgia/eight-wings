"""High-density emplacements, tracked vehicles, theropods and their remains."""
import math
from pixlib import emit, rng_for
from _jobb_hires import Drawing, SCALE, STEEL, SAND, GREEN, RED
from _jobb_anatomy import dinosaur


def pad(size=16):
    d = Drawing(size, size)
    e = size - 2
    d.plate([(3, 1), (e - 2, 1), (e, 3), (e, e - 2), (e - 2, e),
             (3, e), (1, e - 2), (1, 3)], STEEL)
    for x, y in ((3, 3), (e - 2, 3), (3, e - 2), (e - 2, e - 2)):
        d.bolt(x, y)
    for side in (-1, 1):
        x = 2.1 if side < 0 else e - 1.1
        d.line(x, 4, x, e - 3, 'g1')
        d.line(x + .25, 4, x + .25, e - 3, 'g4')
    d.seam([(4, 2.1), (e - 4, 2.1), (e - 2, 4.1)], STEEL)
    d.seam([(4, e - .8), (e - 4, e - .8)], STEEL)
    return d


def barrel(d, cx, cy, angle, length=6, width=1.5):
    dx, dy = math.sin(angle), -math.cos(angle)
    nx, ny = -dy, dx
    def pt(distance, side):
        return cx + distance * dx + side * nx, cy + distance * dy + side * ny
    d.plate([pt(-1, -width), pt(length, -width), pt(length, width), pt(-1, width)], STEEL)
    d.line(*pt(0, -.5), *pt(length - 1, -.5), 'g4')
    for dist in (1, 3, length):
        d.line(*pt(dist, -width), *pt(dist, width), 'g1')
    d.line(*pt(length - .5, -.6), *pt(length - .5, .6), 'ink')
    d.line(*pt(-.5, width - .25), *pt(length - 1, width - .25), 'c2')
    for dist in (.4, 1.2, 2):
        d.line(*pt(dist, -width + .3), *pt(dist, width - .3), 'g2')
    d.line(*pt(2.25, -width + .3), *pt(2.25, width - .3), 'g4')


def turret(f):
    d = pad()
    d.oval(7.5, 7.5, 5.5, 5.5, STEEL)
    d.oval(7.5, 7, 4.5, 4.5, SAND)
    for i in range(8):
        a = i * math.pi / 4
        d.point(7.5 + 4.6 * math.sin(a), 7.5 + 4.6 * math.cos(a), 'g4')
    barrel(d, 7.5, 7, f * math.pi / 4)
    d.oval(7.5, 7, 1.7, 1.7, SAND)
    d.line(6.5, 6.5, 8, 6.5, 's5')
    d.rect(4, 10, 1, .5, 'r2')
    d.point(4, 10, 'r4')
    for i in range(12):
        a = i * math.pi / 6
        x, y = 7.5 + 4.9 * math.sin(a), 7.5 + 4.9 * math.cos(a)
        d.line(x, y, x + .25 * math.sin(a), y + .25 * math.cos(a), 's1')
    d.seam([(4.7, 6), (4.7, 8.7), (5.5, 9.4)], SAND)
    d.bolt(5, 8.7, 's5')
    return d.finish(True)


def bunker(f):
    d = pad()
    d.panel(3, 3, 9, 9, SAND)
    d.panel(4.5, 4.5, 6, 6, STEEL)
    if f < 2:
        for x in (5, 7.5):
            d.panel(x, 5, 2.5, 5, RED)
            d.line(x + .5, 5.5, x + .5, 9, 'r4')
        d.line(7.5, 5, 7.5, 10, 'ink')
        d.rect(6.5, 7, 2, .5, 's5')
    else:
        d.rect(5, 5, 5, 5, 'r1')
        d.rect(5.5, 5.5, 4, 4.5, 'o2')
        d.rect(6, 6, 3, 4, 'o3')
        d.rect(6.5, 6.5, 2, 3, 'y2')
        d.rect(7, 7, 1, 2, 'white')
        d.panel(3.5, 5, 1.5, 5, RED)
        d.panel(10, 5, 1.5, 5, RED)
    for x in (3, 11):
        d.vent(x, 6, 1, 4, 's2')
    for x in range(4, 12):
        d.line(x, 12, x - .5, 13, 'y1' if x % 2 else 'ink')
    d.oval(11.5, 2.5, 1, 1, RED)
    d.point(11, 2, 'white' if f == 1 else 'r2')
    for x in (3.7, 10.7):
        d.pipe([(x, 3.5), (x, 5)], STEEL, 4)
        d.bolt(x, 10.8)
        d.line(x - .3, 11.2, x + .3, 11.2, 's1')
    d.line(5, 4.2, 10, 4.2, 'g1')
    for x in (5.3, 6.5, 8.5, 9.7):
        d.rect(x, 3.3, .5, .5, 's2')
        d.point(x, 3.3, 's5')
    if f < 2:
        for x in (5.7, 8.3):
            d.line(x, 6, x, 8.6, 'r2')
            d.line(x + .25, 6, x + .25, 8.6, 'r4')
            d.bolt(x, 9)
    return d.finish(True)


def tank(f):
    d = Drawing(16, 16)
    for x in (1.5, 11):
        d.panel(x, 2, 3, 11, STEEL)
        d.rect(x + .5, 2.5, 2, 10, 'g1')
        for y in range(round(2.5 * SCALE) + f * 2, round(12.5 * SCALE), 5):
            d.c.hline(round((x + .5) * SCALE), round((x + 2) * SCALE), y, 'g3')
            d.c.set(round((x + 1) * SCALE), y + 1, 'g2')
    d.panel(4.5, 3, 6.5, 9, GREEN)
    d.vent(5.5, 3.5, 4.5, 2, 'n2')
    d.oval(7.75, 7.75, 3.4, 3.8, SAND)
    d.panel(6, 6, 3.5, 3, SAND)
    d.oval(7.5, 6.75, 1, 1, STEEL)
    barrel(d, 7.75, 8, math.pi, 5.5, 1)
    for x in (5, 10):
        d.rect(x, 11, .5, .5, 'r4')
        d.bolt(x, 4)
    for x in (2, 11.5):
        for y in (3, 5.5, 8, 10.5):
            d.point(x, y, 'g4')
            d.point(x + 1.2, y + .5, 'g2')
    d.seam([(5, 9.2), (5, 10.5), (6, 11.2)], GREEN)
    d.seam([(10.1, 4.1), (10.1, 5.5)], GREEN)
    d.oval(9.5, 9.2, .6, .6, STEEL)
    d.line(9.5, 9.2, 10.5, 7.5, 'g1')
    return d.finish(True)


def silo(f):
    d = pad()
    d.oval(7.5, 7.5, 5.6, 5.6, SAND)
    d.oval(7.5, 7.5, 4.7, 4.7, ('ink', 'ink', 'r1', 'r1', 'r2', 'r3'))
    gap = (0, 1.5, 3, 4.5)[f]
    # Two circular sliding doors, clipped to the original hatch aperture.
    for y in range(3 * SCALE, 12 * SCALE + 1):
        for x in range(3 * SCALE, 12 * SCALE + 1):
            u, v = (x / SCALE - 7.5) / 4.5, (y / SCALE - 7.5) / 4.5
            if u*u + v*v < 1 and abs(x / SCALE - 7.5) >= gap:
                d.c.set(x, y, 'g4' if y < 4.5 * SCALE else 'g3' if x < 7.5 * SCALE else 'g2')
    if f == 0:
        d.line(7.5, 3.5, 7.5, 11.5, 'ink')
        d.line(4, 7, 6.5, 7, 'y1')
        d.line(8.5, 7, 11, 7, 'y1')
    else:
        d.glass(7.5, 7, min(gap - .3, 1.5), 3.2, RED)
        d.line(7, 5, 7, 9, 'white')
        d.line(6.5, 10, 8.5, 10, 'r4')
    d.point(3, 9, 'r3')
    d.bolt(11, 4)
    for i in range(8):
        a = i * math.pi / 4
        d.bolt(7.5 + 5.1 * math.cos(a), 7.5 + 5.1 * math.sin(a), 's5')
    for side in (-1, 1):
        xx = 7.5 + side * max(gap + .5, 1)
        d.line(xx, 5.1, xx, 9.8, 'g1')
        d.line(xx + .25, 5.1, xx + .25, 9.8, 'g4')
    return d.finish(True)


def fort(f):
    d = pad(32)
    # Corner slab seams surround the raised gun platform.
    for side in (-1, 1):
        def xx(v): return v if side < 0 else 31 - v
        for y in (3, 26):
            d.line(xx(3), y, xx(8), y, 'g1')
            d.line(xx(3), y - .5, xx(8), y - .5, 'g4' if side < 0 else 'g3')
            d.line(xx(3), y, xx(3), y + 2, 'g1')
        for y in (5, 24):
            d.bolt(xx(3), y)
    d.plate([(9, 4), (22, 4), (27, 10), (27, 22), (22, 27),
             (9, 27), (4, 22), (4, 10)], SAND)
    for x in (5.5, 23):
        d.panel(x, 8, 3, 13)
        d.vent(x + .5, 10, 2, 8, 'r2')
        d.bolt(x + .5, 8.5)
        d.bolt(x + .5, 19)
    d.panel(10, 5, 11, 4, STEEL)
    d.rect(11, 6, 9, 2, 'r1')
    for x in range(12, 20, 2):
        d.rect(x, 6.5, .5, 1, 'o3')
    d.oval(15.5, 15, 7.5, 7.5, STEEL)
    d.oval(15.5, 14.5, 6, 6, SAND)
    d.panel(11, 10, 9, 7, STEEL)
    d.glass(15.5, 11.5, 3, 1, RED)
    d.vent(13, 14, 5, 2.5, 'g2')
    for x in (11, 17.5):
        d.panel(x, 16, 3, 11, STEEL)
        d.line(x + .5, 17, x + .5, 24, 'g4')
        for y in (18, 21, 24):
            d.line(x, y, x + 2.5, y, 'g1')
        d.rect(x + .5, 25, 2, 1.5, 'y2' if f else 'r1')
        if f:
            d.point(x + 1, 25.5, 'white')
    for x in (9, 21):
        d.pipe([(x, 22), (x, 25), (x + 2, 27)], SAND)
        d.bolt(x, 8)
    for x in range(10, 23):
        d.line(x, 28, x - .5, 29, 'y1' if x % 2 else 'ink')
    for side in (-1, 1):
        def x(v): return v if side < 0 else 31 - v
        for xx, yy in ((5, 5), (8, 24), (12, 8)):
            d.bolt(x(xx), yy)
        d.seam([(x(7.5), 6), (x(9.5), 4.7), (x(13), 4.7)], SAND)
        d.seam([(x(7.8), 22), (x(10), 24), (x(10), 26)], SAND)
        for y in (18.8, 19.5, 21.8, 22.5):
            d.line(x(11.5), y, x(12.5), y, 'g4')
        d.pipe([(x(9), 10), (x(9), 18)], STEEL, 4)
    d.oval(15.5, 18.4, 1, 1.5, RED)
    d.line(15.1, 17.7, 15.1, 18.5, 'r4')
    return d.finish(True)


def radar(f):
    d = pad()
    d.panel(4.5, 9, 6, 3, GREEN)
    d.panel(6.5, 5, 2, 6, STEEL)
    a = f * math.pi / 4
    def pt(along, across):
        return 7.5 + along * math.cos(a) - across * math.sin(a), 6 + along * math.sin(a) + across * math.cos(a)
    d.plate([pt(-5.5, -1), pt(-4, 2.5), pt(4, 2.5), pt(5.5, -1),
             pt(3.5, -2.5), pt(-3.5, -2.5)], GREEN)
    for along in (-3, -1.5, 0, 1.5, 3):
        d.line(*pt(along, -2), *pt(along, 2), 'n1')
    d.line(*pt(-4.5, 0), *pt(4.5, 0), 'n4')
    d.line(*pt(0, 0), *pt(0, -3.5), 'g4')
    d.point(*pt(0, -3.5), 'y2')
    d.rect(10.5, 11, .5, 1, 'r4' if f % 2 else 'r1')
    for along in (-3, -1.5, 0, 1.5, 3):
        for across in (-1.2, .3, 1.5):
            d.point(*pt(along, across), 'n4')
            d.point(*pt(along + .25, across + .25), 'n2')
    d.line(*pt(-4, -1), *pt(4, -1), 'n2')
    d.bolt(5, 10)
    d.rect(7, 10, 2, .7, 'g1')
    d.point(7.5, 10, 'r4' if f % 2 else 'y1')
    return d.finish(True)


def rubble(size):
    d = Drawing(size, size)
    rng = rng_for('g_rubble_s' if size == 16 else 'g_rubble_l')
    mid = size / 2
    contour = []
    for i in range(20):
        a = i * math.tau / 20
        r = mid * rng.uniform(.68, .86)
        contour.append((mid + math.cos(a) * r, mid + math.sin(a) * r * .75))
    d.flat(contour, 'g1')
    # Torn structural beams and folded sheets stay grouped around the impact centre.
    for i in range(6 if size == 16 else 14):
        a = i * 2.4
        radius = (i / (6 if size == 16 else 14)) ** .5 * mid * .55
        x, y = mid + math.cos(a) * radius, mid + math.sin(a) * radius * .75
        d.plate([(x - 2, y - 1), (x + .4, y - 1.5), (x + 2, y),
                 (x + .7, y + 1.8), (x - 1.7, y + 1)],
                ('ink', 'g1', 's1', 'g2', 'g3', 's3'))
        d.flat([(x - 1.7, y + .2), (x + .5, y - .3), (x + .8, y + 1.5)], 'ink')
        d.line(x - 1.6, y - .9, x, y - 1.2, 'g4')
        d.point(x + .7, y + 1.3, 's2')
    for side in (-1, 1):
        d.pipe([(mid + side * 2, mid - 2), (mid + side * 4, mid - 1),
                (mid + side * 5, mid + 2)], STEEL, 4)
    d.oval(mid, mid, 1.6, 1.2, ('ink', 'ink', 'g1', 's1', 'g2', 's2'))
    return d.finish(True)


def generate():
    for name, frames, draw in (('g_turret', 8, turret), ('g_bunker', 3, bunker),
                               ('g_tank', 2, tank), ('g_silo', 4, silo),
                               ('g_fort', 2, fort), ('g_radar', 4, radar)):
        for f in range(frames):
            emit(name, draw(f), frame=f, group='ground')
    for size, name in ((20, 'g_raptor'), (32, 'g_rex')):
        for f in range(4):
            emit(name, dinosaur(size, f), frame=f, group='ground')
        emit('g_bones_s' if size == 20 else 'g_bones_l', dinosaur(size, bones=True), group='ground')
    emit('g_rubble_s', rubble(16), group='ground')
    emit('g_rubble_l', rubble(32), group='ground')


if __name__ == '__main__':
    generate()
