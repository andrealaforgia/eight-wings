"""Downward-facing enemies redrawn at four texels per logical pixel."""
import math
from _e6_render import preserve_emit as emit
from _e6_enemies import generate as generate_round6
from _jobb_hires import Drawing, SCALE, STEEL, RED, GOLD, GREEN, PURPLE


def drone(f):
    d = Drawing(16, 16)
    reach = (1, 1.5, 2.5, 1.5)[f]
    d.plate([(reach, 3), (4, 4.5), (6, 6), (5, 10), (1.5, 12), (2, 8)], GREEN, True)
    for x in (3, 12):
        d.panel(x - .5, 6, 1.5, 4, STEEL)
        d.line(x, 7, x, 9, 'n4')
    d.oval(7.75, 7.5, 4.5, 5.8, GREEN)
    d.plate([(7.75, 2), (6, 4), (6, 10), (7.75, 12), (9.5, 10), (9.5, 4)], GREEN)
    d.glass(7.75, 6.5, 2.8, 2, GOLD)
    d.line(5.5, 9.5, 6.5, 11, 'n1')
    d.line(10, 9.5, 9, 11, 'n1')
    d.panel(6.5, 12, 2.5, 1.5)
    for x in (6.5, 8.5):
        d.rect(x, 12.5, .5, 1.5, 'ink')
    for i in range(3):
        d.point(3 + i * .5, 10.5 - i, 'y2' if i == f % 3 else 'n1')
        d.point(12.5 - i * .5, 10.5 - i, 'o3' if i == f % 3 else 'n1')
    d.bolt(5, 4)
    d.point(11, 9, 's3')
    for side in (-1, 1):
        x = 7.75 + side * 2.8
        d.seam([(x, 4.2), (x + side * .6, 6), (x + side * .4, 9.4)], GREEN)
        d.vent(x - .5, 8.3, .9, 2, 'n2')
        d.pipe([(7.75 + side * 3, 6), (7.75 + side * 5, 5)], STEEL, 4)
        d.bolt(x, 4.5)
    d.panel(7.1, 9.5, 1.3, 2, STEEL)
    d.line(7.5, 10, 7.5, 11, 'o3')
    return d.finish()


def swoop(f):
    d = Drawing(16, 16)
    tip = (1, 1.5, 2, 1.5)[f]
    d.flame(7.75, 4, (3, 3.5, 2.5, 2)[f], f)
    d.plate([(1, tip), (4.5, 4), (7.8, 6), (7.8, 12.5),
             (4.5, 10), (2, 13), (1, 9), (2.5, 6)], RED, True)
    for side in (-1, 1):
        def x(v): return 7.75 + side * v
        d.line(x(6), 3.5, x(3), 7, 'r4' if side < 0 else 'r3')
        d.line(x(5), 7, x(4), 10, 'r1')
        d.rect(x(4.5), 8, .5, 2, 'g4')
    d.plate([(6.5, 5), (7.75, 4), (9, 5), (9, 11), (7.75, 14.5), (6.5, 11)], RED)
    d.glass(7.75, 7, 1.4, 2, GOLD)
    d.line(7.25, 10, 7.75, 13, 'r4')
    d.bolt(4, 5.5)
    d.bolt(10.5, 5.5)
    for side in (-1, 1):
        def x(v): return 7.75 + side * v
        d.plate([(x(5.6), 4.5), (x(2), 6.2), (x(2), 9.5),
                 (x(4.7), 8.4), (x(5), 6.8)], RED)
        d.seam([(x(5.8), 3.5), (x(3.2), 5.7), (x(3.5), 8.4), (x(5.3), 10.5)], RED)
        d.vent(x(4.2) - .5, 6.3, 1.1, 1.7, 'r2')
        d.panel(x(5.4) - .4, 9.2, .8, 2.1, STEEL)
        d.rect(x(5.4) - .2, 10.7, .4, .5, 'ink')
        for i in range(3):
            d.point(x(4.5 - i * .7), 5.4 + i * .4, 'g4')
        d.bolt(x(2.3), 8.8, 'r4')
    d.rect(7.3, 10.5, .8, 1.8, 'r1')
    d.line(7.3, 10.5, 7.3, 11.7, 'o3')
    return d.finish()


def diver(f):
    d = Drawing(16, 16)
    d.flame(7.75, 4, (3, 2.5, 3.5, 3)[f], f)
    d.plate([(1, 2.5), (6, 5.5), (7.75, 3.5), (7.75, 14.5),
             (2.5, 8.5), (4.5, 8.5)], GOLD, True)
    d.plate([(7.75, 4), (6.5, 6), (6.5, 10), (7.75, 14.5), (9, 10), (9, 6)], GOLD)
    d.line(2, 4, 6, 7, 'y1')
    d.line(13.5, 4, 10, 7, 'o3')
    d.glass(7.75, 6.5, 1.2, 1.7, RED)
    d.line(7.25, 9, 7.75, 13, 'y2')
    for x in (4, 10):
        d.vent(x, 6.5, 1.5, 1.5, 'r2')
    for y in (8, 9.5, 11):
        d.point(8.5, y, 'r1')
    d.rect(7, 2.5, 1.5, 1, ('o3', 'y1', 'white', 'r4')[f])
    for side in (-1, 1):
        def x(v): return 7.75 + side * v
        d.seam([(x(6), 3.5), (x(2.4), 6.6), (x(2), 9.8)], GOLD)
        d.plate([(x(4.9), 4.3), (x(3.1), 5.5), (x(2.6), 7.9), (x(3.8), 7)], GOLD)
        d.line(x(4.5), 5, x(3.2), 6, 's1')
        d.line(x(4.5), 5.25, x(3.2), 6.25, 'y1')
        d.bolt(x(2.5), 8, 'y2')
        for y in (10, 11, 12):
            d.line(x(.6), y, x(.9), y + .3, 'o1')
    return d.finish()


def spinner(f):
    d = Drawing(16, 16)
    for i in range(4):
        a = (i + f / 4) * math.pi / 2
        x, y = 7.75 + 6 * math.cos(a), 7.75 + 6 * math.sin(a)
        d.pipe([(7.75, 7.75), (x, y)], STEEL, 3)
        d.oval(x, y, 1.3, 1.3, PURPLE)
        d.point(x - .5, y - .5, 'p4')
    d.oval(7.75, 7.75, 5, 5, PURPLE)
    d.oval(7.75, 7.75, 3.8, 3.8, STEEL)
    d.oval(7.75, 7.75, 3, 3, PURPLE)
    for i in range(8):
        a = i * math.pi / 4
        d.point(7.75 + math.cos(a) * 4.2, 7.75 + math.sin(a) * 4.2,
                'y2' if (i - f * 2) % 8 < 2 else 'p1')
    d.glass(7.75, 7.5, 2, 2, RED)
    d.line(7, 6.5, 8.5, 8.5, 'm2')
    d.point(7, 6.5, 'white')
    for i in range(12):
        a = i * math.pi / 6 + f * math.pi / 8
        x, y = 7.75 + 3.4 * math.cos(a), 7.75 + 3.4 * math.sin(a)
        d.line(x, y, x + .55 * math.cos(a), y + .55 * math.sin(a), 'p1')
        d.point(x - .25, y - .25, 'p4')
    d.curve([(3.8, 6.5), (4.8, 3), (8, 3), (10, 4)], 'white')
    return d.finish()


def wing(f):
    d = Drawing(16, 16)
    d.flame(7.75, 4, 3 if f else 2.5, f)
    d.plate([(1, 7), (4, 5.5), (6.5, 3), (7.75, 5), (7.75, 14.5),
             (5.5, 11), (3.5, 9), (1, 10)], PURPLE, True)
    d.flat([(1.5, 7), (5, 6), (6, 8), (3, 8)], 'm2')
    d.flat([(14, 7), (10.5, 6), (9.5, 8), (12.5, 8)], 'm1')
    for x in (2, 12):
        d.panel(x, 8.5, 1.5, 3)
        d.rect(x + .5, 10.5, .5, 1, 'ink')
    d.plate([(6.5, 4), (7.75, 3), (9, 4), (9.5, 10), (7.75, 14.5), (6, 10)], PURPLE)
    d.glass(7.75, 6.5, 1.5, 2, RED)
    d.line(7, 10, 7.5, 12, 'p4')
    d.rect(6.5, 3, 2.5, 1, 'y1' if f else 'r3')
    d.bolt(4.5, 6)
    d.bolt(10.5, 6)
    for side in (-1, 1):
        def x(v): return 7.75 + side * v
        d.seam([(x(5.9), 7.5), (x(3), 6.7), (x(2.7), 9.2)], PURPLE)
        d.plate([(x(4.8), 7.7), (x(3.3), 7.2), (x(3.5), 8.8), (x(4.8), 9.1)], PURPLE)
        d.vent(x(3.6) - .5, 8, .9, 1.5, 'm1')
        d.pipe([(x(1.8), 9), (x(4), 9.5)], STEEL, 3)
        d.bolt(x(2), 10, 'p4')
        d.line(x(1), 10, x(.5), 12.7, 'm1')
    d.rect(7.3, 10, .7, 1.3, 'm2')
    return d.finish()


def bomber(f):
    d = Drawing(32, 24)
    for x in (10.5, 21):
        d.flame(x, 5, 3.5 if f else 3, f)
    d.plate([(1.5, 6), (5, 3.5), (10, 6), (13, 9), (15.75, 5),
             (15.75, 21.5), (10, 18), (2, 18), (.5, 14)], RED, True)
    for x in (3, 23):
        d.panel(x, 7, 5, 10, STEEL)
        d.panel(x + .5, 7.5, 4, 3, RED)
        d.vent(x + 1, 12, 3.5, 3)
        d.panel(x + 1, 15.5, 2, 5, STEEL)
        d.rect(x + 1.5, 19, 1, 1.5, 'ink')
        d.bolt(x + .5, 10.5)
    for x in (9, 20):
        d.panel(x, 4, 2.5, 9)
        d.vent(x + .5, 6, 1.5, 4, 'r2')
        d.rect(x + .5, 4, 1.5, 1, 'y2' if f else 'o3')
    d.plate([(14, 6), (15.75, 4), (17.5, 6), (19.5, 15), (17.5, 20),
             (15.75, 22), (14, 20), (12, 15)], RED)
    d.glass(15.75, 10, 2, 3, GOLD)
    d.line(14, 15, 15.5, 17, 'r4')
    d.line(15.5, 17, 15.5, 20, 'r4')
    d.line(17.5, 14, 18, 17, 'r1')
    d.vent(14, 2.5, 3.5, 1.5, 'o2')
    for x in (12.5, 18.5):
        d.bolt(x, 13)
        d.point(x, 18, 'o3')
    for side in (-1, 1):
        def x(v): return 15.75 + side * v
        d.plate([(x(13.5), 6.5), (x(10), 5), (x(7), 8),
                 (x(7.7), 12), (x(13), 12)], RED)
        d.seam([(x(13.5), 8), (x(11.5), 6.7), (x(8), 9), (x(8.5), 12)], RED)
        d.seam([(x(13), 15.7), (x(8), 16), (x(5), 18), (x(3.7), 19)], RED)
        for y in (7, 8.5, 10):
            d.bolt(x(9), y, 'r4')
        d.vent(x(6) - .6, 12.5, 1.3, 3, 'r2')
        d.panel(x(12) - .75, 16, 1.5, 3, STEEL)
        d.rect(x(12) - .25, 18.5, .5, 1, 'ink')
        d.line(x(3), 12, x(2.8), 16, 'r1')
        d.line(x(2.5), 12, x(2.2), 16, 'r4')
        for yy in (13, 14, 15):
            d.point(x(4), yy, 'g4')
    d.panel(14.5, 17.5, 2.5, 2.5, STEEL)
    d.rect(15, 18, 1.5, 1.5, 'r1')
    d.line(15, 18, 16, 18, 'y1' if f else 'r3')
    return d.finish()


def saucer(f):
    d = Drawing(24, 16)
    d.oval(11.75, 8.5, 10.8, 5.5, PURPLE)
    d.oval(11.75, 7.5, 9.5, 4.4, STEEL)
    d.oval(11.75, 7, 8.5, 3.8, PURPLE)
    for x in (4, 18):
        d.panel(x, 10, 2, 3, STEEL)
        d.rect(x + .5, 12, 1, 1, 'r1')
    d.glass(11.75, 5.5, 4.8, 3.8, GREEN)
    d.line(11.5, 2.5, 11.5, 6, 'n1')
    d.line(8, 7, 15.5, 7, 'n1')
    d.panel(10.5, 10, 2.5, 4)
    d.rect(11, 13, 1.5, 1, 'r4')
    for i, x in enumerate((3, 5.5, 8, 15.5, 18, 20.5)):
        y = 9.5 if i in (0, 5) else 11
        d.rect(x, y, 1, .5, 'y2' if (i - f) % 4 == 0 else 'r2')
    d.point(4, 7, 'white')
    for side in (-1, 1):
        def x(v): return 11.75 + side * v
        d.curve([(x(9.6), 7.2), (x(10.4), 10.5), (x(6), 13), (x(2), 12.5)], 'p1')
        for i in range(6):
            a = .15 + i * .22
            xx, yy = x(9 * math.cos(a)), 8.5 + 4.2 * math.sin(a)
            d.line(xx, yy, xx - side * .6, yy - .5, 'g1')
            d.point(xx - .2, yy - .25, 'g4')
        d.panel(x(5.5) - .5, 9.2, 1, 1.5, PURPLE)
    d.curve([(8.3, 3.4), (9.3, 1.6), (12, 1.3), (13.5, 2.5)], 'n4')
    return d.finish()


def orb(f):
    d = Drawing(6, 6)
    d.oval(2.75, 2.75, 2.7, 2.7, ('ink', 'm1', 'm2', 'r4', 'y2', 'white'))
    d.oval(2.75, 2.75, (1.3, 1.65, 1.8, 1.5)[f], (1.3, 1.65, 1.8, 1.5)[f],
           ('m1', 'm2', 'o3', 'y2', 'white', 'white'))
    d.rect(2, 2, 1.5, 1.5, 'white')
    for x, y in ((1, 2.5), (4.5, 2.5), (2.5, 1), (2.5, 4.5)):
        d.point(x, y, ('m2', 'o3', 'r4', 'y2')[f])
    return d.finish()


def missile(f):
    d = Drawing(6, 12)
    d.flame(2.75, 4, 3.5 if f else 3, f)
    d.plate([(2.75, 10.5), (1.5, 8.5), (1.5, 4), (4, 4), (4, 8.5)], RED)
    d.plate([(.5, 4.5), (1.5, 5), (1.5, 8), (.5, 8.5)], RED)
    d.plate([(5, 4.5), (4, 5), (4, 8), (5, 8.5)], RED)
    d.line(2, 4.5, 2, 8, 'white')
    d.line(3.5, 5, 3.5, 8, 'r4')
    d.line(2, 8.5, 3.5, 8.5, 'g1')
    d.point(2.5, 10, 'y2')
    d.line(2, 5, 3.5, 5, 'r1')
    d.line(2, 6, 3.5, 6, 'r1')
    d.rect(2.25, 7, 1, .5, 'y1')
    d.point(2.25, 9, 'white')
    return d.finish()


def beam(f):
    d = Drawing(8, 16)
    colours = ('ink', 'm1', 'm1', 'm2', 'r4', 'y2', 'white', 'white',
               'white', 'white', 'y2', 'r4', 'm2', 'm1', 'm1', 'ink')
    for y in range(d.c.h):
        for x in range(d.c.w):
            colour = colours[x * len(colours) // d.c.w]
            if 0 < y < d.c.h - 1 and (y + f * SCALE * 2) % (SCALE * 4) < 2:
                if x in (5, d.c.w - 6):
                    colour = 'o3'
            d.c.set(x, y, colour)
    return d.finish()


def generate():
    for name, count, draw in (
        ('e_drone', 4, drone), ('e_swoop', 4, swoop), ('e_diver', 4, diver),
        ('e_spinner', 4, spinner), ('e_wing', 2, wing), ('e_bomber', 2, bomber),
        ('e_saucer', 4, saucer), ('bullet_e', 4, orb), ('bullet_m', 2, missile), ('beam', 2, beam),
    ):
        for f in range(count):
            emit(name, draw(f), frame=f, group='enemies')

    generate_round6()


if __name__ == '__main__':
    generate()
