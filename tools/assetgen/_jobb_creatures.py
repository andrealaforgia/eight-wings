"""Hand-composed top-down theropods and matching articulated skeletons."""
from pixlib import Canvas


def dinosaur(size, frame=0, bones=False):
    large = size == 32
    c = Canvas(size, size)
    # Shared anatomical landmarks keep each skeleton recognisably its creature.
    cx = size // 2
    hip, shoulder, nose = (15, 20, 28) if large else (9, 12, 17)
    width = 5 if large else 3
    swing = (-1, 1, 1, -1)[frame]
    tail = [(cx, hip), (cx - 1, hip - 5), (cx + swing * 2, 3), (cx + swing * 3, 1)]
    dark, low, mid, high = ('r1', 'r2', 'o2', 's3') if large else ('n1', 'n2', 'n3', 'n4')
    if bones:
        for a, b in zip(tail, tail[1:]):
            c.line(*a, *b, 's4')
        for y in range(3, shoulder + 1, 2):
            c.rect(cx - 1, y, 3, 1, 's5')
            c.set(cx - 1, y, 'white')
        for y in range(hip - 3, shoulder, 2):
            rib = width - abs(y - hip) // 3
            for side in (-1, 1):
                c.line(cx + side, y, cx + side * rib, y + 1, 'g4')
                c.line(cx + side * rib, y + 1, cx + side * (rib - 1), y + 3, 's5')
        for side in (-1, 1):
            knee = (cx + side * (width + 2), hip + 1)
            foot = (cx + side * (width + 1), hip + (6 if large else 4))
            c.line(cx, hip, *knee, 's5')
            c.line(*knee, *foot, 'g4')
            c.hline(foot[0] - 1, foot[0] + 1, foot[1], 'white')
        c.ellipse(cx, nose - 3, width, 4 if large else 2, 's4')
        c.ellipse(cx - 1, nose - 4, width - 1, 2, 's5')
        c.hline(cx - width + 1, cx + width - 1, nose - 5, 'white')
        for side in (-1, 1):
            c.rect(cx + side * (width - 2) - 1, nose - 4, 2, 2, 'ink')
            for x in range(cx + side * 2, cx + side * width, side):
                c.set(x, nose - 1, 'white')
        c.set(cx, nose - 2, 'ink')
    else:
        for side in (-1, 1):
            step = (0, 2, 0, -2)[frame] * side
            x = cx + side * width
            c.polygon([(cx, hip - 2), (x, hip - 2), (x + side * 3, hip + 1 + step),
                       (x + side, hip + 3 + step), (cx, hip + 2)], low)
            c.line(x, hip, x + side * 2, hip + 1 + step, high if side < 0 else mid)
            if large:
                c.ellipse(x + side, hip + step, 3, 3, low)
                c.line(x + side - 1, hip - 1 + step, x + side, hip + 1 + step, mid)
            c.rect(x + min(0, side), hip + 2 + step, 2, 3, dark)
            for claw in (-1, 1):
                c.set(x + claw, hip + 5 + step, 's5')
        c.polygon([(cx - width + 1, hip), (cx - 2, hip - 5), tail[2], tail[3],
                   (cx + 2, hip - 5), (cx + width - 1, hip)], low)
        c.line(cx - 1, hip - 3, *tail[2], mid)
        c.ellipse(cx, hip, width, 5 if large else 3, low)
        c.ellipse(cx - 1, hip - 1, width - 1, 4 if large else 2, mid)
        for side in (-1, 1):
            c.line(cx + side * 2, shoulder - 1, cx + side * (width + 1), shoulder + 1, dark)
            c.set(cx + side * (width + 1), shoulder + 2, 's5')
        c.rect(cx - 2, shoulder - 1, 5, 5, low)
        c.ellipse(cx, nose - 3, width + (1 if large else 0), 4 if large else 3, dark)
        c.ellipse(cx - 1, nose - 4, width, 3 if large else 2, mid)
        c.hline(cx - width + 1, cx, nose - 6 if large else nose - 5, high)
        c.hline(cx - width + 1, cx + width - 1, nose - 1, low)
        for side in (-1, 1):
            c.set(cx + side * (width - 1), nose - 4, 'y2')
            c.set(cx + side * (width - 1), nose - 3, 'ink')
            c.set(cx + side * 2, nose - 1, 's5')
        c.hline(cx - 1, cx + 1, nose - 2, dark)
        for y in range(hip - 5, shoulder, 2):
            c.set(cx, y, 'o3' if not large else 'r4')
            c.set(cx + 1, y + 1, dark)
        if large:
            # Paired brow plates and a distinct lower jaw break up the broad skull.
            c.hline(cx - 4, cx - 1, nose - 6, 's4')
            c.hline(cx + 1, cx + 4, nose - 6, 'o2')
            c.vline(cx - 4, nose - 5, nose - 3, 's3')
            c.hline(cx - 3, cx + 3, nose, 'r2')
            c.hline(cx - 2, cx + 2, nose - 1, 'r1')
            c.set(cx - 2, nose - 1, 's5')
            c.set(cx + 2, nose - 1, 's4')
            c.set(cx - 2, nose - 3, 'r1')
            c.set(cx + 2, nose - 3, 'r1')
            for x, y in ((cx - 3, hip - 2), (cx - 2, hip + 1),
                         (cx + 3, hip - 1), (cx + 2, hip + 2)):
                c.hline(x, x + 1, y, 'r2')
                c.set(x, y - 1, 's3' if x < cx else 'o2')
        else:
            c.set(cx - 2, hip - 1, 'n4')
            c.set(cx + 2, hip + 1, 'n1')
            c.set(cx - 1, nose - 4, 's5')
        c.set(cx + width - 1, hip + 1, 'o3' if large else 's3')
        c.outline('ink')
    result = Canvas(size, size)
    for y in range(size):
        for x in range(size):
            if c.get(x, y)[3] and (x + y) % 2 == 0:
                result.set(x + 1, y + 1, 'g1')
    result.blit(c, 0, 0)
    return result
