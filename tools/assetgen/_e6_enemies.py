"""Four original downward-facing Round 6 aircraft."""
import math
from pixlib import emit
from _e6_render import Art, TEAL, STEEL, RED, PURPLE, BONE, FIRE
from _jobb_hires import GOLD, GREEN


def weaver(frame):
    d = Art(16, 16)
    cx = 7.875
    beat = (0, .65, 1.25, .65)[frame]
    # Crescent tips sweep forward; both wings change sweep around a fixed fuselage.
    d.plate([(cx, 5), (5, 5.5), (2.3, 4+beat), (.6, 3+beat),
             (1.1, 7.5+beat), (3.3, 10.4), (6, 9.2), (cx, 10)], TEAL, True)
    for side in (-1, 1):
        x = lambda v: cx+side*v
        d.seam([(x(6.7), 5+beat), (x(5.6), 7.6+beat*.5), (x(2.4), 8.2)], TEAL)
        d.plate([(x(6.5), 5+beat), (x(5.9), 7.4), (x(3.4), 8.4),
                 (x(4.7), 6.7)], GREEN)
        d.panel(x(2.8)-.4, 6, .8, 4, STEEL)
        d.vent(x(2.8)-.25, 6.5, .5, 1.5, 'teal5')
        d.line(x(2.8), 9.1, x(2.8), 10.2, 'ink', 2)
        d.bolt(x(5.5), 8.3, 'teal6')
        d.point(x(5.9), 7.8, 'y2' if frame % 2 else 'n4')
    d.flame(cx, 4, (2.8, 3.5, 2.5, 3.1)[frame], frame)
    d.plate([(cx, 3), (6.6, 5), (6.8, 11.3), (cx, 14.8),
             (9, 11.3), (9.1, 5)], TEAL)
    d.glass(cx, 8.6, .85, 2, GOLD)
    d.line(7.2, 11, 7.8, 13.3, 'n4')
    d.point(8.6, 12, 'teal5')
    return d.finish()


def hunter(frame):
    d = Art(16, 16)
    cx = 7.875
    d.flame(cx, 5.1, (3, 4, 2.8, 3.5)[frame], frame)
    # Deep split swept fins and a long forward needle distinguish this pursuit dart.
    d.plate([(cx, 6.4), (5.3, 5), (1.1, 2.8), (2.4, 8.1),
             (5.9, 10.3), (cx, 12.4)], RED, True)
    for side in (-1, 1):
        x = lambda v: cx+side*v
        d.seam([(x(5.8), 4.3), (x(4.6), 7.1), (x(1.8), 9.3)], RED)
        d.vent(x(3.1)-.45, 6.6, .9, 1.8, 'o2')
        d.line(x(5.3), 4.9, x(4.1), 7, 's4')
        d.bolt(x(3.6), 7.9, 'r4')
    d.plate([(cx, 3.4), (6.5, 6), (7.1, 11.8), (cx, 15),
             (8.7, 11.8), (9.25, 6)], STEEL)
    d.oval(cx, 8.3, 1.55, 1.8, RED)
    d.oval(cx, 8.3, (1, 1.1, .8, .95)[frame], 1.25, FIRE)
    d.line(cx, 7.7, cx, 8.8, 'y2', 2)
    d.point(cx-.4, 7.8, 'white')
    d.line(7.4, 11, 7.8, 14, 'r4')
    d.line(8.5, 11.5, 8.1, 13.6, 'r2')
    return d.finish()


def gunship(frame):
    d = Art(32, 24)
    cx = 15.875
    for x in (9.6, 22.15):
        d.flame(x, 6, 3.3 if frame else 2.7, frame)
    d.plate([(cx, 6), (11, 4.2), (5, 3.6), (1, 6), (.6, 14),
             (3.4, 17.6), (11, 18.8), (cx, 21.1)], STEEL, True)
    for side in (-1, 1):
        x = lambda v: cx+side*v
        d.plate([(x(13.4), 6), (x(10), 4.6), (x(5), 6.5),
                 (x(5), 13), (x(12.7), 13.8)], RED)
        d.seam([(x(12.7), 7), (x(9.4), 6.2), (x(6.3), 7.6), (x(6.3), 11.8)], RED)
        d.vent(x(10.2)-1.3, 8, 2.6, 3.1, 'red_l')
        d.panel(x(5.7)-1.3, 3.6, 2.6, 8, STEEL)
        d.vent(x(5.7)-.8, 4.4, 1.6, 3, 'r2')
        d.panel(x(9.4)-1.6, 12, 3.2, 9.2, STEEL)
        d.panel(x(9.4)-1.1, 14, 2.2, 3.4, RED)
        for yy in (15, 16, 18.4):
            d.line(x(9.4)-1, yy, x(9.4)+1, yy, 'g1')
        d.oval(x(9.4), 20, 1.3, 1.1, RED if not frame else FIRE)
        d.rect(x(9.4)-.6, 20, 1.2, .6, 'ink' if not frame else 'y2')
        if frame:
            d.point(x(9.4)-.3, 20, 'white')
        for yy in (7, 12, 16):
            d.bolt(x(13.1), yy)
        d.line(x(12.7), 14.6, x(11.8), 15.5, 'y1')
        d.line(x(12.1), 14.6, x(11.2), 15.5, 'y1')
    d.plate([(cx, 4.7), (12.5, 8), (11.5, 16), (13, 20),
             (cx, 22), (18.8, 20), (20.2, 16), (19.3, 8)], RED)
    d.vent(14.2, 6.5, 3.4, 3.2, 'red_l')
    d.glass(cx, 15.8, 3.2, 3.8, TEAL)
    d.line(cx, 12.2, cx, 19.3, 'g1', 2)
    d.curve([(13.4, 16.4), (14, 15.7), (17.7, 15.7), (18.4, 16.4)], 'g2', 2)
    d.bolt(12.4, 11)
    d.bolt(19.4, 11)
    return d.finish()


def splitter(frame):
    d = Art(20, 20)
    cx = 9.875
    d.oval(cx, 10, 7.4, 8.8, PURPLE)
    d.oval(cx, 10.8, (3.5, 3.9, 4.2, 3.8)[frame], 5, PURPLE)
    d.oval(cx, 11.8, (1.3, 1.7, 2, 1.6)[frame], 2.4, FIRE)
    # Six separate shell lobes leave dark seams and an exposed glowing core.
    for side in (-1, 1):
        x = lambda v: cx+side*v
        d.plate([(x(.3), 1.4), (x(3.8), 2.1), (x(6.3), 5),
                 (x(5.9), 7.6), (x(2.2), 7), (x(.6), 5.4)], BONE)
        d.plate([(x(6.7), 6.4), (x(7.3), 10.2), (x(6.1), 14),
                 (x(3), 13.2), (x(2.6), 9.3)], PURPLE)
        d.plate([(x(5.9), 14.2), (x(4.6), 17.2), (x(1), 18.2),
                 (x(.8), 15.7), (x(2.6), 13.8)], BONE)
        d.seam([(x(1.1), 2.6), (x(3.4), 3.5), (x(4.9), 5.5)], BONE)
        d.seam([(x(6.2), 8.4), (x(5.7), 10.4), (x(4.7), 11.6)], PURPLE)
        for yy in (9, 10.5, 12):
            d.point(x(5.1), yy, 'y2' if (int(yy*2)+frame)%4 == 0 else 'm2')
        d.bolt(x(3.4), 5, 'bone6')
        d.bolt(x(3), 16.1, 'bone6')
    d.plate([(cx-1.2, 17), (cx, 19), (cx+1.2, 17)], PURPLE)
    d.point(cx-.5, 11.1, 'white')
    return d.finish()


def generate():
    for name, count, draw in (('e_weaver', 4, weaver), ('e_hunter', 4, hunter),
                               ('e_gunship', 2, gunship), ('e_splitter', 4, splitter)):
        for frame in range(count):
            emit(name, draw(frame), frame=frame, group='enemies')
