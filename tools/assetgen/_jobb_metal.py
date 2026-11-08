"""Small native-pixel mechanical details shared by Job B2."""
from palette import PALETTE


def panel(c, x, y, w, h, ramp=('g1', 'g2', 'g3', 'g4')):
    dark, shade, mid, light = ramp
    c.rect(x, y, w, h, dark)
    c.rect(x + 1, y + 1, w - 2, h - 2, mid)
    c.hline(x + 1, x + w - 2, y, light)
    c.vline(x, y + 1, y + h - 2, light)
    c.hline(x + 1, x + w - 2, y + h - 2, shade)
    c.vline(x + w - 2, y + 1, y + h - 2, shade)


def vent(c, x, y, w, h, glow='r2'):
    c.rect(x, y, w, h, 'ink')
    for yy in range(y + 1, y + h, 2):
        c.hline(x + 1, x + w - 2, yy, glow)
    c.hline(x, x + w - 1, y - 1, 'g4')


def lit_symmetry(c):
    c.mirror_left_to_right()
    shades = {PALETTE[a]: PALETTE[b] for a, b in
              (('white', 'g4'), ('g4', 'g3'), ('r4', 'r3'), ('n4', 'n3'),
               ('p4', 'p3'), ('y2', 'y1'), ('c4', 'c3'), ('s5', 's4'))}
    for y in range(c.h):
        for x in range(c.w // 2, c.w):
            p = c.get(x, y)
            if p in shades:
                c.set(x, y, shades[p])
    c.outline('ink')
    return c
