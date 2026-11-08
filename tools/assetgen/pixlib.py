"""Shared pixel-art toolkit for the Eight Wings asset generators.

Pure standard library (no Pillow). All art is drawn pixel by pixel onto small
RGBA canvases using ONLY colours from palette.py, then written as PNG files.
Everything here is deterministic: the same seed always produces the same pixels.
"""
import os
import random
import struct
import zlib

from palette import PALETTE, TRANSPARENT


class Canvas:
    """A w*h RGBA canvas. Colours are palette names (str), (r,g,b,a) tuples, or None."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [TRANSPARENT] * (w * h)

    @staticmethod
    def _rgba(c):
        if c is None:
            return TRANSPARENT
        if isinstance(c, str):
            return PALETTE[c]
        return tuple(c)

    def inside(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def set(self, x, y, c):
        x, y = int(x), int(y)
        if self.inside(x, y):
            self.px[y * self.w + x] = self._rgba(c)

    def get(self, x, y):
        if self.inside(x, y):
            return self.px[y * self.w + x]
        return TRANSPARENT

    def rect(self, x, y, w, h, c):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.set(xx, yy, c)

    def hline(self, x0, x1, y, c):
        for x in range(min(x0, x1), max(x0, x1) + 1):
            self.set(x, y, c)

    def vline(self, x, y0, y1, c):
        for y in range(min(y0, y1), max(y0, y1) + 1):
            self.set(x, y, c)

    def line(self, x0, y0, x1, y1, c):
        dx, dy = abs(x1 - x0), -abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx + dy
        while True:
            self.set(x0, y0, c)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x0 += sx
            if e2 <= dx:
                err += dx
                y0 += sy

    def disc(self, cx, cy, r, c):
        for y in range(int(cy - r - 1), int(cy + r + 2)):
            for x in range(int(cx - r - 1), int(cx + r + 2)):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r + 0.25:
                    self.set(x, y, c)

    def ellipse(self, cx, cy, rx, ry, c):
        for y in range(int(cy - ry - 1), int(cy + ry + 2)):
            for x in range(int(cx - rx - 1), int(cx + rx + 2)):
                if ((x - cx) / max(rx, 0.5)) ** 2 + ((y - cy) / max(ry, 0.5)) ** 2 <= 1.0:
                    self.set(x, y, c)

    def polygon(self, pts, c):
        """Filled polygon (even-odd) with integer pixel centres."""
        ys = [p[1] for p in pts]
        for y in range(int(min(ys)), int(max(ys)) + 1):
            xs = []
            n = len(pts)
            for i in range(n):
                (x0, y0), (x1, y1) = pts[i], pts[(i + 1) % n]
                if (y0 <= y < y1) or (y1 <= y < y0):
                    xs.append(x0 + (y - y0) * (x1 - x0) / (y1 - y0))
            xs.sort()
            for i in range(0, len(xs) - 1, 2):
                for x in range(int(round(xs[i])), int(round(xs[i + 1]))):
                    self.set(x, y, c)

    def blit(self, other, ox, oy):
        for y in range(other.h):
            for x in range(other.w):
                p = other.px[y * other.w + x]
                if p[3]:
                    self.set(ox + x, oy + y, p)

    def copy(self):
        c = Canvas(self.w, self.h)
        c.px = list(self.px)
        return c

    def flip_h(self):
        c = Canvas(self.w, self.h)
        for y in range(self.h):
            for x in range(self.w):
                c.px[y * self.w + x] = self.px[y * self.w + (self.w - 1 - x)]
        return c

    def flip_v(self):
        c = Canvas(self.w, self.h)
        for y in range(self.h):
            for x in range(self.w):
                c.px[y * self.w + x] = self.px[(self.h - 1 - y) * self.w + x]
        return c

    def mirror_left_to_right(self):
        """Copy the left half onto the right half, mirrored (bilateral symmetry)."""
        for y in range(self.h):
            for x in range(self.w // 2):
                self.px[y * self.w + (self.w - 1 - x)] = self.px[y * self.w + x]

    def rotated(self, quarter_turns):
        c = self.copy()
        for _ in range(quarter_turns % 4):
            n = Canvas(c.h, c.w)
            for y in range(c.h):
                for x in range(c.w):
                    n.px[x * n.w + (c.h - 1 - y)] = c.px[y * c.w + x]
            c = n
        return c

    def outline(self, colour, include_diagonals=False):
        """Add a 1px outline (in `colour`) around all opaque pixels, on transparent neighbours."""
        src = self.copy()
        nb = [(1, 0), (-1, 0), (0, 1), (0, -1)]
        if include_diagonals:
            nb += [(1, 1), (-1, -1), (1, -1), (-1, 1)]
        for y in range(self.h):
            for x in range(self.w):
                if src.get(x, y)[3] == 0:
                    for dx, dy in nb:
                        if src.get(x + dx, y + dy)[3]:
                            self.set(x, y, colour)
                            break

    def replace(self, old, new):
        o, n = self._rgba(old), self._rgba(new)
        self.px = [n if p == o else p for p in self.px]

    def opaque_count(self):
        return sum(1 for p in self.px if p[3])

    def save_png(self, path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        raw = bytearray()
        for y in range(self.h):
            raw.append(0)
            for x in range(self.w):
                raw.extend(self.px[y * self.w + x])

        def chunk(tag, data):
            body = tag + data
            return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

        png = b"\x89PNG\r\n\x1a\n"
        png += chunk(b"IHDR", struct.pack(">IIBBBBB", self.w, self.h, 8, 6, 0, 0, 0))
        png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        png += chunk(b"IEND", b"")
        with open(path, "wb") as f:
            f.write(png)


def rng_for(name, seed=1985):
    """A deterministic random.Random for a sprite name."""
    return random.Random("%s:%d" % (name, seed))


def out_dir():
    """Where generators write: assets/generated (override with TC_OUT)."""
    here = os.path.dirname(os.path.abspath(__file__))
    return os.environ.get("TC_OUT", os.path.join(here, "..", "..", "assets", "generated"))


def emit(name, canvas, frame=None, group=None):
    """Write one sprite. Single sprites: <name>.png ; animation frames: <name>.<n>.png"""
    fname = name + ("" if frame is None else ".%d" % frame) + ".png"
    sub = group or "misc"
    canvas.save_png(os.path.normpath(os.path.join(out_dir(), sub, fname)))
