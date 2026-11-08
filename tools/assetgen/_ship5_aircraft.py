"""One master airframe, sliced at overlapping docking ribs. Stdlib only."""
from functools import lru_cache
from pixlib import Canvas, emit, rng_for
from _joba_materials import ACCENTS

SCALE = 4
SIZE = 256
CENTRE = 128
# Logical centres and footprints are the game's fixed attachment contract.
LAYOUT = ((0, 0, 16, 24), (-12, 2, 12, 16), (12, 2, 12, 16),
          (0, -16, 12, 12), (0, 15, 16, 10), (-22, 6, 12, 14),
          (22, 6, 12, 14), (-10, -12, 8, 10), (10, -12, 8, 10))


def point(x, y):
    return round(CENTRE + x*SCALE), round(CENTRE + y*SCALE)


def polygon(c, points, colour):
    c.polygon([point(x, y) for x, y in points], colour)


def line(c, a, b, colour):
    c.line(*point(*a), *point(*b), colour)


def ellipse(c, x, y, rx, ry, colour):
    c.ellipse(*point(x, y), rx*SCALE, ry*SCALE, colour)


def paired(points):
    return points + [(-x, y) for x, y in reversed(points)]


# Author left-hand geometry once. These overlap along structural ribs rather
# than leaving the one-logical-pixel air gaps of the old independent pods.
CORE = paired([(0, -11.7), (-1.5, -10.5), (-6.9, -11.1),
               (-7.7, -9.4), (-6.8, -7.4), (-2.8, -6.3), (-3, -3.8),
               (-7.7, -.4), (-7.7, 7.6), (-4, 6.4), (-2.8, 11.4), (0, 11.4)])
INNER = [(-6.2, -5.8), (-9, -4.5), (-17.8, .8), (-17.8, 8.3),
         (-13, 8.7), (-9, 6.5), (-6.2, 7.1)]
OUTER = [(-16.2, -.6), (-20.8, 2), (-27.7, 6.2), (-27.5, 10.6),
         (-24.4, 12.2), (-21, 10), (-16.2, 8.7)]
CANARD = [(-6.2, -12.2), (-8.2, -15.2), (-9.5, -16.7),
          (-11.1, -15.8), (-13.7, -8.2), (-10.5, -7.3), (-6.2, -9)]
NOSE = paired([(0, -21.7), (-.7, -20.2), (-1.8, -17.5),
               (-3, -15.2), (-4.8, -11.4), (-2.4, -10.2), (0, -10.2)])
TAIL = paired([(0, 10.2), (-3, 10.2), (-5, 11.6), (-7.5, 10.5),
               (-7.2, 16.4), (-5.5, 17.3), (0, 16.3)])
SHAPES = (CORE, INNER, [(-x, y) for x, y in INNER], NOSE, TAIL,
          OUTER, [(-x, y) for x, y in OUTER],
          CANARD, [(-x, y) for x, y in CANARD])


def plate(c, points, base='g3'):
    """Thin directional bevels, with broad quiet metal faces."""
    mask = Canvas(SIZE, SIZE)
    polygon(mask, points, 'white')
    for y in range(SIZE):
        for x in range(SIZE):
            if not mask.get(x, y)[3]:
                continue
            colour = base
            if not mask.get(x+1, y)[3] or not mask.get(x, y+1)[3]:
                colour = 'g1'
            elif not mask.get(x-1, y)[3] or not mask.get(x, y-1)[3]:
                colour = 'white'
            elif not mask.get(x-2, y)[3] or not mask.get(x, y-2)[3]:
                colour = 'g4'
            elif not mask.get(x+3, y)[3] or not mask.get(x, y+3)[3]:
                colour = 'g2'
            c.set(x, y, colour)


def rivet(c, x, y):
    px, py = point(x, y)
    c.set(px, py, 'g1')
    c.set(px-1, py-1, 'white')


def exhaust(c, x, y, width, frame):
    plate(c, [(x-width/2-.2, y-1.3), (x+width/2+.2, y-1.3),
              (x+width/2, y+.3), (x-width/2, y+.3)], 'g2')
    for i in range(round(width*2)):
        offset = x-width/2 + .25+i*.5
        line(c, (offset, y-1), (offset, y-.2), 'ink')
    # The dark envelope never moves; only the hot interior flickers.
    polygon(c, [(x-width/2, y), (x+width/2, y),
                (x+width*.35, y+1.4), (x, y+2.3),
                (x-width*.35, y+1.4)], 'r1')
    for inset, colour in enumerate(('o1', 'o2', 'o3', 'y1', 'y2')):
        w = width*(.43-inset*.065)
        length = 1.95-inset*.23
        polygon(c, [(x-w, y+.1), (x+w, y+.1),
                    (x+.15*(frame*2-1), y+length)], colour)
    line(c, (x-.3, y+.15), (x+.3, y+.15), 'white' if frame else 'y1')


@lru_cache(maxsize=2)
def master(frame):
    c = Canvas(SIZE, SIZE)
    # A single outer surface supplies identical colour and panel trajectories
    # on both sides of every crop seam.
    for shape in SHAPES:
        polygon(c, shape, 'g3')
    hull = c.copy()
    for y in range(SIZE):
        for x in range(SIZE):
            if not hull.get(x, y)[3]:
                continue
            if not hull.get(x+1, y)[3] or not hull.get(x, y+1)[3]:
                c.set(x, y, 'g1')
            elif not hull.get(x-1, y)[3] or not hull.get(x, y-1)[3]:
                c.set(x, y, 'white')
            elif not hull.get(x-3, y)[3] or not hull.get(x, y-3)[3]:
                c.set(x, y, 'g4')
            elif not hull.get(x+4, y)[3] or not hull.get(x, y+4)[3]:
                c.set(x, y, 'g2')
    for side in (-1, 1):
        def pts(points):
            return [(side*x, y) for x, y in points]
        def stroke(a, b, colour):
            line(c, (side*a[0], a[1]), (side*b[0], b[1]), colour)
        # Continuous swept spar and blue trailing facets cross wing boundaries.
        polygon(c, pts([(3, .8), (27.2, 8.2), (24.5, 11.8),
                        (20.5, 9), (12.8, 7.5), (4, 5.7)]), 'c1')
        polygon(c, pts([(4, 1.6), (25.8, 8.7), (24.3, 10.8),
                        (14, 6.5), (4, 4.9)]), 'c2')
        stroke((4, 1), (26.7, 8.2), 'g4')
        stroke((4, 1.25), (26.5, 8.45), 'g1')
        stroke((7, -4.8), (27.1, 6.6), 'g4')
        stroke((7.2, -4.4), (26.9, 6.9), 'g2')
        polygon(c, pts([(10, -2.5), (17, 1.3), (16.6, 2.5), (11, -.2)]), 'g4')
        stroke((10.5, -2), (16.6, 1.4), 'white')
        stroke((11, -.1), (16.5, 2.7), 'g2')
        for x, y in ((10.8, 2.5), (21.7, 7.2)):
            stroke((x, y), (x+1, y+.3), 'g1')
            stroke((x, y+.5), (x+.6, y+.7), 'g4')
        # Recessed radiators and forward-facing gun rails share one profile.
        for x, y, length in ((13.2, -.2, 6), (24.8, 5.3, 4.8)):
            plate(c, pts([(x-.8, y), (x+.6, y), (x+1, y+length),
                           (x-.9, y+length)]), 'g4')
            stroke((x-.25, y+.3), (x-.25, y+length-.6), 'white')
            stroke((x-.5, y+.3), (x+.4, y+.3), 'ink')
            for j in range(3):
                stroke((x-.45, y+2+j*.65), (x+.6, y+2+j*.65), 'g1')
        for x, y in ((7, 2.8), (17, 5.9), (21.5, 7), (11, -10.1)):
            rivet(c, side*x, y)
        # Docking clamps sit on the overlap ribs, clear of the swept spar.
        for x, y in ((7.1, 5.5), (17, 6.5), (7.1, -10.5)):
            plate(c, pts([(x-.6, y-.65), (x+.6, y-.65),
                           (x+.6, y+.65), (x-.6, y+.65)]), 'g2')
            stroke((x-.3, y-.3), (x+.3, y-.3), 'c4')
        # Canards have an aerodynamic leading rim and paired gun muzzles.
        polygon(c, pts([(7, -12), (9.7, -15.4), (12.8, -8.6),
                        (10.5, -8), (8.6, -10)]), 'c1')
        stroke((8.7, -14), (11.4, -8.9), 'g4')
        for x in (9.2, 10.2):
            stroke((x, -15.5), (x, -13.4), 'g4')
            stroke((x-.2, -15.5), (x+.2, -15.5), 'ink')
        for x, y, w in ((13.5, 7.4, 1.6), (24.5, 10, 1.5),
                        (10.3, -9.5, 1.2), (4.9, 16.6, 2.5)):
            exhaust(c, side*x, y, w, frame)
    # Symmetric fuselage over the broad wing root: one uninterrupted spine.
    fuselage = Canvas(SIZE, SIZE)
    half = [(0, -11.7), (-1.4, -10), (-2.3, -6), (-2.8, 1),
            (-2.1, 11.4), (0, 11.4)]
    plate(fuselage, paired(half), 'g4')
    fuselage.mirror_left_to_right()
    c.blit(fuselage, 0, 0)
    polygon(c, [(.8, -9), (2, -5), (2.4, 2), (1.7, 8), (.8, 8.6)], 'g2')
    line(c, (-1.1, -10), (-1.8, -3), 'white')
    # Layered curved glass, coloured reflections, two fine structural hoops.
    ellipse(c, 0, -1.5, 1.75, 5.1, 'ink')
    for x, y, rx, ry, colour in ((0, -1.65, 1.5, 4.7, 'c1'),
                                (-.2, -2, 1.2, 4.1, 'c2'),
                                (-.4, -2.6, .8, 3.3, 'c3'),
                                (-.6, -3.5, .35, 1.8, 'c4')):
        ellipse(c, x, y, rx, ry, colour)
    line(c, (-.7, -5.1), (-.9, -3.8), 'white')
    line(c, (.8, .1), (1, 1.2), 'b3')
    line(c, (-1.4, -.5), (1.4, -.5), 'c1')
    line(c, (-.9, 3.8), (.9, 3.8), 'g1')
    for y in (5, 5.8, 6.6):
        line(c, (-.8, y), (.8, y), 'g2')
        line(c, (-.8, y+.25), (.6, y+.25), 'white')
    exhaust(c, 0, 9, 2.1, frame)
    # Nose cannon grows directly out of the same keel and tapers into it.
    plate(c, [(0, -21.7), (-.65, -20.3), (-1.1, -12),
              (0, -10.5), (1.1, -12), (.65, -20.3)], 'g4')
    line(c, (-.25, -20), (-.5, -12.5), 'white')
    line(c, (-.4, -20.8), (.4, -20.8), 'ink')
    for y in (-17.8, -13):
        line(c, (-.75, y), (.75, y), 'g2')
    for side in (-1, 1):
        exhaust(c, side*3.2, -12.7, .9, frame)
    # Tail's centre socket and the wing engine row use the same steel collars.
    plate(c, [(-1.5, 10.5), (1.5, 10.5), (2, 14.8),
              (0, 16), (-2, 14.8)], 'g3')
    line(c, (-.7, 11), (-.7, 14.4), 'white')
    for y in (12, 13, 14):
        line(c, (-.3, y), (1, y), 'g1')
    # Accents are narrow identification flashes, never whole coloured pods.
    stripes = ([(7.8, -3.9), (9, -3.2), (10.5, .9), (9.3, .5)],
               [(2.5, -15.1), (3, -14.4), (4, -11.7), (3.2, -11.5)],
               [(3, 11.4), (4, 11.9), (6.7, 13), (6.6, 13.8)],
               [(19, 1.7), (20, 2.1), (21.7, 6), (20.6, 5.7)],
               [(11.5, -12.2), (12, -10.8), (12.9, -8.7), (12, -8.9)])
    for number, side, index in ((1,-1,0),(2,1,0),(3,-1,1),(3,1,1),
                                (4,-1,2),(4,1,2),(5,-1,3),(6,1,3),
                                (7,-1,4),(8,1,4)):
        shape = [(side*x, y) for x, y in stripes[index]]
        polygon(c, shape, ACCENTS[number-1][2])
        line(c, shape[0], shape[-1], ACCENTS[number-1][3])
        line(c, shape[1], shape[2], ACCENTS[number-1][1])
    return c


@lru_cache(maxsize=18)
def piece(number, frame=0):
    x, y, w, h = LAYOUT[number]
    source = master(frame)
    mask = Canvas(SIZE, SIZE)
    polygon(mask, SHAPES[number], 'white')
    # Nozzles belong to their craft and remain stable across flicker frames.
    engines = {0: ((0,9,2.1),), 1: ((-13.5,7.4,1.6),),
               2: ((13.5,7.4,1.6),), 3: ((-3.2,-12.7,.9),(3.2,-12.7,.9)),
               4: ((-4.9,16.6,2.5),(4.9,16.6,2.5)),
               5: ((-24.5,10,1.5),), 6: ((24.5,10,1.5),),
               7: ((-10.3,-9.5,1.2),), 8: ((10.3,-9.5,1.2),)}
    for ex, ey, ew in engines[number]:
        polygon(mask, [(ex-ew/2-.3,ey-1.3),(ex+ew/2+.3,ey-1.3),
                       (ex+ew/2+.3,ey+2.5),(ex-ew/2-.3,ey+2.5)], 'white')
    out = Canvas(w*SCALE, h*SCALE)
    ox, oy = point(x-w/2, y-h/2)
    for yy in range(out.h):
        for xx in range(out.w):
            if mask.get(ox+xx, oy+yy)[3]:
                out.set(xx, yy, source.get(ox+xx, oy+yy))
    out.outline('ink')
    return out


def core(frame=0):
    return piece(0, frame).copy()


def modules(frame=0):
    return [piece(n, frame).copy() for n in range(1, 9)]


def bank(left=True):
    source = core()
    out = Canvas(source.w, source.h)
    direction = -1 if left else 1
    for y in range(out.h):
        for x in range(out.w):
            sx = round(31.5 + (x-31.5)/.76 + direction*(y-48)*.035)
            out.set(x, y, source.get(sx, y))
    return out


def generate_aircraft():
    for frame in range(2):
        emit('ship_core', core(frame), frame, group='ship')
        for number, sprite in enumerate(modules(frame), 1):
            # There is no stochastic texture; retain the canonical seed contract.
            rng_for('ship_c'+str(number))
            emit('ship_c'+str(number), sprite, frame, group='ship')
    emit('ship_core_bank_l', bank(), group='ship')
    emit('ship_core_bank_r', bank(False), group='ship')


if __name__ == '__main__':
    generate_aircraft()
