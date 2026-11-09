"""Round 6 discrete material shading and geometry, in logical coordinates."""
import math
import os
from pixlib import Canvas, emit, out_dir
from _jobb_hires import Drawing, SCALE

TEAL = ('ink', 'teal1', 'teal2', 'teal3', 'teal4', 'teal5', 'teal6', 's5')
OLIVE = ('ink', 'olive1', 'olive2', 'olive3', 'olive4', 'olive5', 'olive6', 's5')
RUST = ('ink', 'rust1', 'rust2', 'rust3', 'rust4', 'rust5', 'rust6', 's5')
BONE = ('ink', 'bone1', 'bone2', 'bone3', 'bone4', 'bone5', 'bone6', 'white')
STEEL = ('ink', 'g1', 'g2', 'steel_m', 'g3', 'g4', 'steel_l', 'white')
RED = ('ink', 'r1', 'r2', 'red_m', 'r3', 'red_l', 'r4', 'y2')
PURPLE = ('ink', 'p1', 'p2', 'purple_m', 'p3', 'purple_l', 'p4', 'white')
FIRE = ('ink', 'r1', 'o1', 'o2', 'o3', 'y1', 'y2', 'white')


def preserve_emit(name, canvas, frame=None, group=None):
    """Keep shipped legacy PNGs untouched; allow clean output directories too."""
    filename = name + ('' if frame is None else '.%d' % frame) + '.png'
    if not os.path.exists(os.path.join(out_dir(), group, filename)):
        emit(name, canvas, frame=frame, group=group)


def bezier(points, count=80):
    a, b, c, d = points
    for i in range(count + 1):
        t = i / count
        u = 1 - t
        yield tuple(u**3*a[k] + 3*u*u*t*b[k] + 3*u*t*t*c[k] + t**3*d[k]
                    for k in (0, 1))


class Art(Drawing):
    def oval(self, x, y, rx, ry, ramp=STEEL):
        cx, cy, ax, ay = x*SCALE, y*SCALE, rx*SCALE, ry*SCALE
        for yy in range(math.floor(cy-ay), math.ceil(cy+ay)+1):
            for xx in range(math.floor(cx-ax), math.ceil(cx+ax)+1):
                u, v = (xx-cx)/ax, (yy-cy)/ay
                radius = u*u + v*v
                if radius > 1:
                    continue
                edge = any(((xx+dx-cx)/ax)**2 + ((yy+dy-cy)/ay)**2 > 1
                           for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                light = math.sqrt(max(0, 1-radius))*.72 - u*.48 - v*.52
                tone = min(len(ramp)-1, max(1, int((light+.16)* (len(ramp)-2))))
                if radius > .85 and light < .1:
                    tone = 2
                self.c.set(xx, yy, ramp[0 if edge else tone])

    def plate(self, pts, ramp=STEEL, mirror=False):
        if len(ramp) == 8:
            ramp = tuple(ramp[i] for i in (0, 1, 2, 4, 6, 7))
        super().plate(pts, ramp, mirror)

    def tube(self, points, widths, ramp=STEEL):
        """A smoothly tapered curved volume, with one-texel contour."""
        samples = list(bezier(points))
        # Draw the contour as one polygon rather than overlapping outlined discs.
        left, right = [], []
        for i, (x, y) in enumerate(samples):
            t = i / (len(samples)-1)
            width = widths[0]*(1-t) + widths[1]*t
            a, b = samples[max(0, i-1)], samples[min(len(samples)-1, i+1)]
            dx, dy = b[0]-a[0], b[1]-a[1]
            length = max(.001, math.hypot(dx, dy))
            nx, ny = -dy/length*width, dx/length*width
            left.append((x+nx, y+ny)); right.append((x-nx, y-ny))
        contour = left + right[::-1]
        self.plate(contour, ramp)
        mask = Canvas(self.c.w, self.c.h)
        mask.polygon([(x*SCALE, y*SCALE) for x, y in contour], 'white')
        xmin = max(0, math.floor(min(x for x, y in contour)*SCALE))
        xmax = min(self.c.w-1, math.ceil(max(x for x, y in contour)*SCALE))
        ymin = max(0, math.floor(min(y for x, y in contour)*SCALE))
        ymax = min(self.c.h-1, math.ceil(max(y for x, y in contour)*SCALE))
        for yy in range(ymin, ymax+1):
            for xx in range(xmin, xmax+1):
                if not mask.get(xx, yy)[3]:
                    continue
                edge = any(not mask.get(xx+dx, yy+dy)[3]
                           for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                if edge:
                    continue
                x, y = xx/SCALE, yy/SCALE
                i = min(range(len(samples)),
                        key=lambda k: (samples[k][0]-x)**2+(samples[k][1]-y)**2)
                t = i/(len(samples)-1)
                width = max(.05, widths[0]*(1-t)+widths[1]*t)
                u, v = (x-samples[i][0])/width, (y-samples[i][1])/width
                radius = min(1, u*u+v*v)
                light = .76*math.sqrt(1-radius)-.45*u-.5*v
                tone = min(len(ramp)-1, max(1, int((light+.16)*(len(ramp)-2))))
                if light < .08 and radius > .75:
                    tone = 2
                self.c.set(xx, yy, ramp[tone])

    def limb(self, a, b, width, ramp):
        self.tube([a, a, b, b], (width, width*.72), ramp)
        self.oval(*b, width*.75, width*.8, ramp)

    def scales(self, cx, cy, rx, ry, ramp, spacing=1.4):
        # Only mark interior material, so ordered scales never break the contour.
        from palette import PALETTE
        allowed = {PALETTE[n] for n in ramp[2:-1]}
        for row in range(-int(ry/spacing), int(ry/spacing)+1):
            yy = cy + row*spacing
            for col in range(-int(rx/spacing), int(rx/spacing)+1):
                xx = cx + (col + .5*(row % 2))*spacing
                if ((xx-cx)/rx)**2 + ((yy-cy)/ry)**2 > .8:
                    continue
                if self.c.get(round(xx*SCALE), round(yy*SCALE)) not in allowed:
                    continue
                self.line(xx-.35, yy, xx, yy+.25, ramp[2])
                self.line(xx, yy+.25, xx+.35, yy, ramp[3])
                self.point(xx-.25, yy-.25, ramp[5] if xx < cx else ramp[4])

    def bone(self, a, b, width=.5):
        self.limb(a, b, width, BONE)
        self.oval(*a, width*1.05, width*.85, BONE)
        self.line(a[0]-.15, a[1]-.15, b[0]-.15, b[1]-.15, 'bone6')

    def vertebrae(self, points, widths=(.55, .15)):
        self.tube(points, widths, BONE)
        samples = list(bezier(points, 26))
        for i, (x, y) in enumerate(samples[1:-1]):
            w = widths[0]*(1-i/26) + widths[1]*i/26
            self.oval(x, y, w*1.55, .3, BONE)
