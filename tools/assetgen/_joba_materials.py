"""Shared enamel ramps and directional, one-pixel metal bevels."""
from pixlib import Canvas

ACCENTS = (
    ('r1', 'r2', 'r3', 'r4'), ('s2', 'o3', 'y1', 'y2'),
    ('n1', 'n2', 'n3', 'n4'), ('p1', 'p2', 'p3', 'p4'),
    ('o1', 'o2', 'o3', 'y1'), ('c1', 'c2', 'c3', 'c4'),
    ('p1', 'm1', 'm2', 'p4'), ('b1', 'g3', 'g4', 'white'),
)


def plate(c, points, ramp=('b1', 'g2', 'g4', 'white')):
    """Bevel a solid panel; preserve top-left light even on mirrored geometry."""
    mask = Canvas(c.w, c.h)
    mask.polygon(points, 'white')
    for y in range(c.h):
        for x in range(c.w):
            if not mask.get(x, y)[3]:
                continue
            if not mask.get(x + 1, y)[3] or not mask.get(x, y + 1)[3]:
                colour = ramp[0]
            elif not mask.get(x, y - 1)[3] or not mask.get(x - 1, y)[3]:
                colour = ramp[3]
            else:
                colour = ramp[2] if x < c.w // 2 else ramp[1]
            c.set(x, y, colour)


def mirrored(points, width, right):
    return [(width - 1 - x, y) for x, y in points] if right else points


def reflect_lit(c, ramps):
    """Reflect finished pixels, then move vertical bevels back to the lit side.

    Reflecting polygon vertices changes half-open scanline coverage by one pixel;
    reflecting the finished mask instead guarantees matching docking silhouettes.
    """
    c=c.flip_h()
    src=c.copy()
    for ramp in ramps:
        colours={Canvas._rgba(col) for col in ramp}
        for y in range(c.h):
            for x in range(c.w):
                if src.get(x,y) not in colours:
                    continue
                if src.get(x,y-1) not in colours or src.get(x,y+1) not in colours:
                    continue
                if src.get(x-1,y) not in colours:
                    c.set(x,y,ramp[3])
                elif src.get(x+1,y) not in colours:
                    c.set(x,y,ramp[0])
    return c
