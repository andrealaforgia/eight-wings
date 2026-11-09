"""Four-texel weapon ordnance and hexagonal, letter-coded ammunition cases."""
import math

from pixlib import emit, rng_for
from _joba_ultra import canvas, poly, rect, line, ellipse, panel, rivet, sphere

SCALE = 4
FIRE = ('ink', 'r1', 'o1', 'o2', 'o3', 'y1', 'y2', 'white')
WEAPONS = {
    'vulcan': ('V', ('ink', 'b1', 'b2', 'c2', 'b3', 'b4', 'c4', 'white')),
    'spread': ('S', FIRE),
    'laser': ('L', ('ink', 'c1', 'b1', 'c2', 'c3', 'c4', 'y2', 'white')),
    'missile': ('M', ('ink', 'r1', 'r2', 'm1', 'r3', 'r4', 'y2', 'white')),
    'wave': ('W', ('ink', 'n1', 'n2', 'c2', 'n3', 'n4', 'y2', 'white')),
}


def pellet():
    c = canvas(4, 4)
    sphere(c, 1.875, 1.875, 1.55, 1.55, FIRE)
    ellipse(c, 1.7, 1.65, .8, .85, 'y1')
    ellipse(c, 1.5, 1.4, .55, .55, 'y2')
    rect(c, 1.25, 1.0, .5, .5, 'white')
    c.set(11, 11, 'o3')
    c.outline('ink')
    return c


def laser(frame):
    c = canvas(3, 16)
    # The two dark rails hold the envelope fixed while charge knots travel.
    poly(c, [(1.375, .25), (2.5, 1.25), (2.5, 14.5),
             (1.375, 15.5), (.25, 14.5), (.25, 1.25)], 'c1')
    poly(c, [(1.375, .5), (2.25, 1.5), (2.25, 14.25),
             (1.375, 15.25), (.5, 14.25), (.5, 1.5)], 'c2')
    rect(c, .75, 1.75, 1.25, 12.5, 'c3')
    rect(c, 1, 1.25, .75, 13.5, 'c4')
    rect(c, 1, 2, .5, 12, 'white')
    for y in (3 + frame*1.75, 8 + frame*1.75, 12 + frame*1.5):
        ellipse(c, 1.375, y, .875, .6, 'c4')
        rect(c, .75, y-.25, 1.25, .5, 'white')
    c.outline('ink')
    return c


def missile(frame):
    c = canvas(5, 10)
    # Draw one fin and mirror the structure before applying directional light.
    poly(c, [(1.75, 4.75), (.5, 6), (.25, 7.75),
             (1.75, 7.25), (2.375, 6)], 'g2')
    line(c, .5, 6.25, .25, 7.5, 'g4')
    c.mirror_left_to_right()
    panel(c, [(1.5, 2.5), (3.25, 2.5), (3.25, 7.25),
              (2.875, 7.75), (1.875, 7.75), (1.5, 7.25)])
    poly(c, [(2.375, .25), (1.5, 2.25), (1.5, 3),
             (3.25, 3), (3.25, 2.25)], 'r2')
    poly(c, [(2.375, .5), (1.75, 2.25), (1.75, 2.75),
             (2.75, 2.75), (2.75, 2)], 'r3')
    line(c, 2.125, 1.25, 1.875, 2.25, 'r4')
    c.set(8, 7, 'white')
    line(c, 1.75, 3.25, 3, 3.25, 'ink')
    line(c, 1.75, 3.5, 3, 3.5, 'y2')
    line(c, 1.75, 4, 1.75, 6.5, 'white')
    line(c, 3, 4, 3, 6.75, 'c2')
    for y in (4.5, 5, 5.5):
        line(c, 2.25, y, 2.75, y, 'g1')
    rect(c, 1.875, 7, 1, .5, 'g1')
    poly(c, [(1.75, 7.5), (3, 7.5), (3, 8.25),
             (2.375, 9.5), (1.75, 8.25)], 'r1')
    for i, colour in enumerate(('o1', 'o2', 'o3', 'y1', 'y2')):
        width = .55-i*.075
        poly(c, [(2.375-width, 7.5), (2.375+width, 7.5),
                 (2.375+(.125 if frame else -.125), 9.25-i*.25)], colour)
    rect(c, 2.125, 7.5, .5, 1.25 if frame else .25, 'white')
    if not frame:
        rect(c, 2.125, 8, .5, .5, 'o3')
    c.outline('ink')
    return c


def wave(frame):
    c = canvas(10, 6)
    # A swept elliptical shell, with its hollow and hooked tips facing aft.
    for y in range(c.h):
        for x in range(c.w):
            dx = (x/SCALE-4.875)/4.5
            if abs(dx) > 1:
                continue
            top = 4.9-4.25*math.sqrt(1-dx*dx)
            thickness = 1.2+.6*(1-abs(dx))
            distance = y/SCALE-top
            if not 0 <= distance <= thickness or y/SCALE > 5.2:
                continue
            t = distance/thickness
            core = 'white' if (abs(dx) < .4) != bool(frame) else 'y2'
            colour = ('n2' if t < .16 else 'n3' if t < .32 else
                      'n4' if t < .5 else 'white' if t < .7 else
                      'y2' if t < .82 else 'n2')
            if .45 <= t < .75:
                colour = core
            c.set(x, y, colour)
    for x in (1, 8.75):
        line(c, x, 3.5, x, 4.75, 'n4')
    for x in (2.25+frame, 6.5+frame):
        dx = (x-4.875)/4.5
        y = 4.9-4.25*math.sqrt(1-dx*dx)+.5
        ellipse(c, x, y, .4, .3, 'white')
    c.outline('ink')
    return c


# Polygonal strokes keep diagonal glyphs crisp at the native texel density.
LETTERS = {
    'V': [[(0, 0), (1.6, 0), (3, 5.7), (4.4, 0), (6, 0),
           (3.85, 8), (2.15, 8)]],
    'S': [[(1.2, 0), (6, 0), (6, 1.5), (1.6, 1.5), (1.6, 3.15),
           (4.8, 3.15), (6, 4.35), (6, 6.8), (4.8, 8), (0, 8),
           (0, 6.5), (4.4, 6.5), (4.4, 4.75), (1.2, 4.75),
           (0, 3.55), (0, 1.2)]],
    'L': [[(0, 0), (1.75, 0), (1.75, 6.25), (6, 6.25),
           (6, 8), (0, 8)]],
    'M': [[(0, 8), (0, 0), (1.6, 0), (3, 3.3), (4.4, 0),
           (6, 0), (6, 8), (4.5, 8), (4.5, 3.3), (3, 6.2),
           (1.5, 3.3), (1.5, 8)]],
    'W': [[(0, 0), (1.75, 0), (2.1, 5.5), (2.8, 2.75),
           (4.2, 2.75), (4.9, 5.5), (5.25, 0), (7, 0),
           (6, 8), (4.2, 8), (3.5, 5.75), (2.8, 8), (1, 8)]],
}


def weapon_capsule(name, frame):
    letter, ramp = WEAPONS[name]
    c = canvas(16, 16)
    rng = rng_for('wp_'+name)
    # Point-left/right hexagon, deliberately unlike the numbered octagonal pods.
    panel(c, [(4.25, .75), (11.5, .75), (15.25, 7.875),
              (11.5, 15), (4.25, 15), (.5, 7.875)])
    panel(c, [(4.5, 1.5), (11.25, 1.5), (14.375, 7.875),
              (11.25, 14.25), (4.5, 14.25), (1.375, 7.875)], ramp)
    # Thin emissive gasket surrounds a broad, quiet dark face for the letter.
    points = [(4.875, 2.25), (10.875, 2.25), (13.625, 7.875),
              (10.875, 13.5), (4.875, 13.5), (2.125, 7.875)]
    poly(c, points, ramp[5] if frame else ramp[4])
    panel(c, [(5, 2.75), (10.75, 2.75), (13, 7.875),
              (10.75, 13), (5, 13), (2.75, 7.875)],
          ('ink', 'ink', ramp[1], 'g1', ramp[1], 'g1', 'g2', 'g3'))
    for x, y in ((4.5, 1.25), (11.25, 1.25),
                 (4.5, 14.5), (11.25, 14.5)):
        rivet(c, x, y)
    for x in (1.75, 13.5):
        rect(c, x, 7, .5, 1.75, 'ink')
        rect(c, x, 7.25, .5, 1.25, ramp[6] if frame else ramp[4])
    for i in range(3):
        x = 6.25+i*1.25
        rect(c, x, 1.5, .75, .5, 'white' if i == frame else ramp[3])
        rect(c, x, 13.875, .75, .375, ramp[5] if i == 2-frame else ramp[2])
    # Cast shadow then a bevelled ivory glyph; width is 6 logical pixels.
    for polygon in LETTERS[letter]:
        poly(c, [(x+4.75, y+4.125) for x, y in polygon], 'ink')
        panel(c, [(x+4.5, y+3.75) for x, y in polygon],
              ('ink', ramp[2], ramp[3], ramp[5], ramp[6],
               'white', 'white', 'white'))
    # Tiny keyed serial notches are fixed across animation frames.
    for i in range(3):
        x = 6+rng.randrange(0, 10)*.25
        c.set(round(x*SCALE), 49, ramp[3])
    c.outline('ink')
    return c


def generate():
    emit('shot_pellet', pellet(), group='ship')
    for frame in range(2):
        for name, draw in (('laser', laser), ('missile', missile), ('wave', wave)):
            emit('shot_'+name, draw(frame), frame, group='ship')
        for name in WEAPONS:
            emit('wp_'+name, weapon_capsule(name, frame), frame, group='pickups')
