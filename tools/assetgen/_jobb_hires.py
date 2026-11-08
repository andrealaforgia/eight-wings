"""Job B raster geometry in logical coordinates, rendered at four texels/pixel."""
import math

from pixlib import Canvas

SCALE = 4
STEEL = ('ink', 'g1', 'g2', 'g3', 'g4', 'white')
RED = ('ink', 'r1', 'r2', 'r3', 'r4', 'y2')
GOLD = ('ink', 's1', 'o1', 'o2', 'o3', 'y2')
GREEN = ('ink', 'n1', 'n2', 'n3', 'n4', 's5')
PURPLE = ('ink', 'p1', 'p2', 'p3', 'p4', 'white')
TEAL = ('ink', 'b1', 'c1', 'c2', 'c3', 'c4')
SAND = ('ink', 's1', 's2', 's3', 's4', 's5')
BONE = ('g1', 's2', 'g4', 's4', 's5', 'white')
FIRE = ('r1', 'r2', 'o1', 'o2', 'o3', 'y1', 'y2', 'white')


class Drawing:
    def __init__(self, w, h):
        self.c = Canvas(round(w * SCALE), round(h * SCALE))
        self.w, self.h = w, h

    def point(self, x, y, colour):
        self.c.set(round(x * SCALE), round(y * SCALE), colour)

    def line(self, x0, y0, x1, y1, colour, width=1):
        a, b = (round(x0 * SCALE), round(y0 * SCALE)), (round(x1 * SCALE), round(y1 * SCALE))
        for i in range(width):
            self.c.line(a[0] + i, a[1], b[0] + i, b[1], colour)

    def rect(self, x, y, w, h, colour):
        x0, y0, x1, y1 = [round(v * SCALE) for v in (x, y, x + w, y + h)]
        self.c.rect(x0, y0, x1 - x0, y1 - y0, colour)

    def flat(self, pts, colour):
        self.c.polygon([(x * SCALE, y * SCALE) for x, y in pts], colour)

    def plate(self, pts, ramp=STEEL, mirror=False):
        mask = Canvas(self.c.w, self.c.h)
        mask.polygon([(x * SCALE, y * SCALE) for x, y in pts], 'white')
        if mirror:
            mask.mirror_left_to_right()
        occupied = [(x, y) for y in range(mask.h) for x in range(mask.w) if mask.get(x, y)[3]]
        if not occupied:
            return
        xmin, xmax = min(x for x, y in occupied), max(x for x, y in occupied)
        ymin, ymax = min(y for x, y in occupied), max(y for x, y in occupied)
        for x, y in occupied:
            u = (x - xmin) / max(1, xmax - xmin)
            v = (y - ymin) / max(1, ymax - ymin)
            tone = 3 if u * .6 + v * .4 < .68 else 2
            if not mask.get(x, y - 1)[3] or not mask.get(x - 1, y)[3]:
                tone = 5
            elif not mask.get(x, y + 1)[3] or not mask.get(x + 1, y)[3]:
                tone = 1
            elif not mask.get(x, y - 2)[3] or not mask.get(x - 2, y)[3]:
                tone = 4
            elif not mask.get(x, y + 2)[3] or not mask.get(x + 2, y)[3]:
                tone = 2
            self.c.set(x, y, ramp[tone])
        edge = mask.copy()
        edge.outline(ramp[0])
        for y in range(mask.h):
            for x in range(mask.w):
                if edge.get(x, y)[3] and not mask.get(x, y)[3]:
                    self.c.set(x, y, ramp[0])

    def oval(self, x, y, rx, ry, ramp=STEEL):
        cx, cy, ax, ay = x * SCALE, y * SCALE, rx * SCALE, ry * SCALE
        for yy in range(math.floor(cy - ay), math.ceil(cy + ay) + 1):
            for xx in range(math.floor(cx - ax), math.ceil(cx + ax) + 1):
                u, v = (xx - cx) / ax, (yy - cy) / ay
                radius = u * u + v * v
                if radius > 1:
                    continue
                boundary = any(((xx + dx - cx) / ax) ** 2 + ((yy + dy - cy) / ay) ** 2 > 1
                               for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0)))
                if boundary:
                    tone = 0
                else:
                    normal = math.sqrt(max(0, 1 - radius))
                    light = normal * .72 - u * .48 - v * .52
                    tone = 1 if light < .05 else 2 if light < .37 else 3 if light < .7 else 4
                    if light > 1.01:
                        tone = 5
                    # A thin reflected rim separates the shaded material from its outline.
                    if light < .05 and radius > .80:
                        tone = 2
                self.c.set(xx, yy, ramp[tone])

    def panel(self, x, y, w, h, ramp=STEEL):
        bevel = min(.6, w / 4, h / 4)
        self.plate([(x + bevel, y), (x + w - bevel, y), (x + w, y + bevel),
                    (x + w, y + h - bevel), (x + w - bevel, y + h),
                    (x + bevel, y + h), (x, y + h - bevel), (x, y + bevel)], ramp)

    def vent(self, x, y, w, h, colour='g3'):
        self.rect(x, y, w, h, 'ink')
        for yy in range(round(y * SCALE) + 2, round((y + h) * SCALE) - 1, 4):
            self.c.hline(round(x * SCALE) + 1, round((x + w) * SCALE) - 2, yy, colour)
            self.c.hline(round(x * SCALE) + 2, round((x + w) * SCALE) - 2, yy + 1, 'g1')
        self.line(x, y - .25, x + w - .25, y - .25, 'g4')

    def bolt(self, x, y, light='g4'):
        self.c.disc(round(x * SCALE), round(y * SCALE), 1.6, 'g1')
        self.point(x - .25, y - .25, light)
        self.point(x, y, 'g3')
        self.point(x + .25, y + .25, 'ink')

    def glass(self, x, y, rx, ry, ramp=GOLD):
        self.oval(x, y, rx, ry, STEEL)
        self.oval(x, y, max(.3, rx - .35), max(.3, ry - .35), ramp)
        self.curve([(x - rx * .65, y), (x - rx * .7, y - ry * .7),
                    (x, y - ry * .85), (x + rx * .4, y - ry * .45)], ramp[4])
        self.line(x - rx * .4, y - ry * .35, x - rx * .05, y - ry * .65, 'white', 2)
        self.point(x - rx * .4, y - ry * .35, 'white')
        self.line(x + rx * .5, y + ry * .3, x + rx * .3, y + ry * .55, ramp[3])

    def pipe(self, points, ramp=STEEL, width=3):
        for a, b in zip(points, points[1:]):
            for offset in range(width):
                self.line(a[0] + offset / SCALE, a[1], b[0] + offset / SCALE, b[1],
                          ramp[(1, 4, 3, 2, 1)[offset % 5]])

    def curve(self, points, colour, width=1):
        """Cubic Bezier sampled directly onto the final texel grid."""
        a, b, c, d = points
        steps = max(12, round(sum(math.dist(p, q) for p, q in zip(points, points[1:])) * SCALE * 2))
        previous = a
        for i in range(1, steps + 1):
            t, u = i / steps, 1 - i / steps
            p = tuple(u**3 * a[k] + 3*u*u*t*b[k] + 3*u*t*t*c[k] + t**3*d[k] for k in (0, 1))
            self.line(*previous, *p, colour, width)
            previous = p

    def seam(self, points, ramp=STEEL):
        for a, b in zip(points, points[1:]):
            self.line(*a, *b, ramp[1])
            self.line(a[0], a[1] - .25, b[0], b[1] - .25, ramp[4])

    def flame(self, x, y, length, frame=0):
        for i, colour in enumerate(FIRE):
            t = i / len(FIRE)
            reach = length * (1 - .67 * t)
            w = 1.4 * (1 - t)
            self.flat([(x - w, y), (x - w * .65, y - reach * .55),
                       (x + .18 * (frame % 2), y - reach),
                       (x + w * .65, y - reach * .55), (x + w, y)], colour)
        self.line(x - .5, y, x + .5, y, 'white')

    def finish(self, shadow=False):
        if not shadow:
            return self.c
        c = Canvas(self.c.w, self.c.h)
        for y in range(c.h):
            for x in range(c.w):
                if self.c.get(x, y)[3] and (x + y) % 2 == 0:
                    c.set(x + 2, y + 3, 'g1')
        c.blit(self.c, 0, 0)
        return c
