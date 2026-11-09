"""Shared live/fossil landmarks for three large, downward-facing dinosaurs."""
import math
from pixlib import emit, rng_for
from _e6_render import Art, TEAL, OLIVE, RUST, BONE, FIRE, bezier


def anatomy(kind, frame):
    if kind == 'trike':
        size, cx, cy, rx, ry = 40, 19.875, 16.5, 10, 11.2
        neck = [(cx, 23), (cx, 25), (cx, 28), (cx, 30)]
        tail = [(cx, 11), (cx-3, 7), (cx-2, 3), (cx+1.8, 1.3)]
        head = (cx, 33.3)
        rows, reach, weight = (10, 22), 14, 2.5
    elif kind == 'spino':
        size, cx, cy, rx, ry = 44, 21.875, 19.5, 7.3, 10
        neck = [(cx, 26), (cx-.7, 29), (cx+.2, 31), (cx, 33)]
        tail = [(cx, 13), (cx-6, 9), (cx-5, 3), (cx+2.5, 1.2)]
        head = (cx, 34)
        rows, reach, weight = (16.5, 25), 12.6, 1.5
    else:
        size, cx, cy, rx, ry = 56, 26.875, 22, 17.5, 15.2
        neck = [(cx+2, 32), (cx+23, 36), (cx+16, 47), (cx+5, 50.5)]
        tail = [(cx, 13), (cx-9, 6), (cx-3, 2), (cx+8, 1.2)]
        head = (cx+5, 52.1)
        rows, reach, weight = (13.5, 29), 22, 3.2
    legs = []
    phase = (-.4, 1, .4, -1)[frame]
    # Diagonal pairs step together. The body, neck, skull and pelvis never bob.
    for pair, y in enumerate(rows):
        for side in (-1, 1):
            step = phase*side*(1 if pair == 0 else -1)
            root = (cx+side*rx*.66, y)
            knee = (cx+side*(reach-1.3), y+.7+step*1.7)
            foot = (cx+side*reach, y+3.6+step*2.1)
            legs.append((root, knee, foot, weight if pair else weight*.85))
    return size, cx, cy, rx, ry, neck, tail, head, legs


def feet(d, legs, ramp, fossil=False, clawed=False):
    for root, knee, foot, width in legs:
        if fossil:
            d.bone(root, knee, width*.23)
            d.bone(knee, foot, width*.19)
            for toe in (-1, 0, 1):
                d.bone(foot, (foot[0]+toe*width*.65, foot[1]+1.4), .25)
        else:
            d.limb(root, knee, width, ramp)
            d.limb(knee, foot, width*.78, ramp)
            d.oval(*foot, width, width*.85, ramp)
            for toe in (-1, 0, 1):
                xx, yy = foot[0]+toe*width*.62, foot[1]+width*.5
                if clawed:
                    d.plate([(xx-.25, yy-.4), (xx+.35, yy-.3),
                             (xx+toe*.35, yy+1.1)], BONE)
                else:
                    d.oval(xx, yy, .38, .52, BONE)
            for i in range(3):
                d.line(knee[0]-width*.4, knee[1]+i*.5,
                       knee[0]+width*.35, knee[1]+i*.5+.15, ramp[2])


def ribs(d, cx, cy, rx, ry):
    d.bone((cx, cy-ry*.8), (cx, cy+ry*.78), .65)
    count = 14 if rx > 15 else 10
    for i in range(count):
        t = (i+.5)/count
        y = cy-ry*.78+t*ry*1.55
        reach = rx*.9*math.sin(math.pi*(.1+.8*t))
        for side in (-1, 1):
            pts = [(cx+side*.5, y), (cx+side*reach*.7, y-1.6),
                   (cx+side*(reach+1), y+.6), (cx+side*reach*.55, y+2)]
            d.curve(pts, 'bone1', 4)
            d.curve([(x-.15, yy-.15) for x, yy in pts], 'bone5', 2)
            d.point(cx+side*reach*.85-.25, y+.15, 'bone6')
        d.oval(cx, y, .95, .4, BONE)
    for yy in (cy-ry*.6, cy+ry*.55):
        d.bone((cx-rx*.6, yy), (cx+rx*.6, yy+.35), .6)


def trike_head(d, cx, hy, fossil):
    # Scalloped bony frill, broad enough to read at forty logical pixels.
    for i in range(11):
        a = math.pi+i*math.pi/10
        xx, yy = cx+10.3*math.cos(a), hy-5+5.7*math.sin(a)
        d.oval(xx, yy, 1.5, 1.8, BONE)
    d.oval(cx, hy-5.2, 11.2, 6.3, BONE)
    for side in (-1, 1):
        for i in range(4):
            x = cx+side*(3+i*1.9)
            d.curve([(cx+side*1.1, hy-2.6), (x, hy-4.4),
                     (x+side*.5, hy-6.3), (x, hy-8)], 'bone3', 2)
        if fossil:
            d.oval(cx+side*5, hy-5.7, 3, 2.9, ('ink',)*8)
            d.curve([(cx+side*1, hy-3), (cx+side*3, hy-5),
                     (cx+side*6, hy-7.4), (cx+side*8, hy-7.5)], 'bone5', 3)
    ramp = BONE if fossil else RUST
    d.oval(cx, hy-.3, 5.3, 4.3, ramp)
    d.oval(cx, hy+2.8, 2.8, 1.6, ramp)
    for side in (-1, 1):
        d.oval(cx+side*3.5, hy-.2, 1, .8, ('ink',)*8)
        if not fossil:
            d.oval(cx+side*3.5, hy-.2, .65, .5, FIRE)
            d.point(cx+side*3.5, hy-.3, 'ink')
            d.line(cx+side*4.6, hy-1.4, cx+side*2.4, hy-1.7, 'rust6', 2)
        # Paired long brows point towards the charge, not out sideways.
        d.tube([(cx+side*3.2, hy-2), (cx+side*3.4, hy),
                (cx+side*4.5, hy+2), (cx+side*4.2, hy+4.6)], (.85, .05), BONE)
        d.oval(cx+side*1.2, hy+2.3, .3, .4, ('ink',)*8)
    d.tube([(cx, hy+1), (cx-.3, hy+2.3), (cx-.4, hy+3.6),
            (cx, hy+4.2)], (.65, .05), BONE)
    d.curve([(cx-2.1, hy+3.4), (cx-1, hy+4.1), (cx+1, hy+4.1),
             (cx+2.1, hy+3.4)], 'bone1' if fossil else 'rust1', 2)


def sail(d, cx, cy, fossil=False):
    # A slightly canted sail exposes its striped surface in this overhead view.
    points = [(cx, cy-10), (cx-3.5, cy-9), (cx-6, cy-6.5),
              (cx-6.5, cy-2), (cx-5.8, cy+3), (cx-3, cy+7.5), (cx, cy+8.5)]
    if not fossil:
        d.plate(points, FIRE)
    for i in range(10):
        y = cy-9+i*1.8
        t = (y-(cy-10))/18.5
        reach = 6.3*math.sin(math.pi*t)
        a, b = (cx, y+1), (cx-reach, y-.4)
        if fossil:
            d.bone(a, b, .28)
        else:
            d.line(*a, *b, 'rust1', 3)
            d.line(a[0]-.15, a[1]-.25, b[0]-.15, b[1]-.25, 'y1')
    if not fossil:
        d.curve([(cx-3.5, cy-8.8), (cx-7.1, cy-5),
                 (cx-7.2, cy+1), (cx-3, cy+7.5)], 'o3', 2)
        d.line(cx+.3, cy-10, cx+.3, cy+8.3, 'teal1', 2)


def spino_head(d, cx, hy, fossil):
    ramp = BONE if fossil else TEAL
    d.oval(cx, hy-1.5, 3.7, 3.2, ramp)
    d.plate([(cx-2.4, hy-1), (cx-1.5, hy+6.8), (cx-.7, hy+7.9),
             (cx+.7, hy+7.9), (cx+1.5, hy+6.8), (cx+2.4, hy-1)], ramp)
    d.oval(cx, hy+5.8, 1.7, 1.8, ramp)
    for side in (-1, 1):
        d.oval(cx+side*2.3, hy-1.5, 1 if fossil else .75, .9,
               ('ink',)*8 if fossil else FIRE)
        d.point(cx+side*2.3, hy-1.4, 'ink')
        d.curve([(cx+side*3, hy-.2), (cx+side*2.2, hy+1.7),
                 (cx+side*1.5, hy+5), (cx+side*1.2, hy+6.5)],
                'bone1' if fossil else 'teal1', 2)
        for i in range(7):
            y = hy+.8+i*.8
            x = cx+side*(2.2-i*.12)
            d.line(x, y, x-side*.45, y+.35, 'bone6')
        d.oval(cx+side*.65, hy+5.8, .25, .35, ('ink',)*8)
    d.line(cx-.35, hy-.1, cx-.35, hy+4.5, 'bone6' if fossil else 'teal6')


def bronto_head(d, hx, hy, fossil):
    ramp = BONE if fossil else OLIVE
    d.oval(hx, hy, 2.4, 2.1, ramp)
    d.oval(hx, hy+1.1, 1.75, 1, ramp)
    for side in (-1, 1):
        d.oval(hx+side*1.6, hy-.2, .55, .5,
               ('ink',)*8 if fossil else FIRE)
        if not fossil:
            d.point(hx+side*1.6, hy-.2, 'ink')
            d.line(hx+side*.8, hy-1.3, hx+side*2, hy-1, 'olive6')
        d.point(hx+side*.7, hy+1.2, 'ink')
    d.line(hx-1.2, hy+1.7, hx+1.2, hy+1.7, 'bone1' if fossil else 'olive1')


def dinosaur(kind, frame=0, fossil=False):
    size, cx, cy, rx, ry, neck, tail, head, legs = anatomy(kind, frame)
    d = Art(size, size)
    ramp = {'trike': RUST, 'spino': TEAL, 'bronto': OLIVE}[kind]
    feet(d, legs, ramp, fossil, kind == 'spino')
    if fossil:
        d.vertebrae(tail, (1.1, .1))
        ribs(d, cx, cy, rx, ry)
        d.vertebrae(neck, (.7, .45))
    else:
        d.tube(tail, (2.5 if kind != 'bronto' else 3.6, .12), ramp)
        for i, (x, y) in enumerate(bezier(tail, 15)):
            if i % 2:
                d.line(x-.45, y, x+.45, y+.2, ramp[2])
        if kind == 'bronto':
            d.tube(neck, (4, 1.6), ramp)
        d.oval(cx, cy, rx, ry, ramp)
        d.scales(cx, cy, rx, ry, ramp, 1.65 if kind == 'bronto' else 1.35)
        if kind != 'bronto':
            d.tube(neck, (3, 2.3), ramp)
        if kind == 'bronto':
            # Pale flank patches follow the volume and keep the back uncluttered.
            rng = rng_for('g_bronto')
            for side in (-1, 1):
                for i in range(7):
                    y = cy-8+i*2.4
                    x = cx+side*(rx*.72+math.sin(i*.7))
                    d.oval(x, y, .85+rng.random()*.3, 1.1, OLIVE[3:]+OLIVE[-3:])
            d.curve([(cx-10, cy-8), (cx-15, cy-2), (cx-14, cy+7),
                     (cx-8, cy+10)], 'olive5', 2)
        elif kind == 'trike':
            for row in range(5):
                for side in (-1, 1):
                    x, y = cx+side*(2.1+row*.45), cy-7+row*2.8
                    d.plate([(x, y-1), (x-1.4, y), (x-.9, y+1.7),
                             (x+.9, y+1.7), (x+1.4, y)], RUST)
                    d.line(x-.7, y, x, y-.5, 'rust6')
            for side in (-1, 1):
                for row in range(4):
                    d.oval(cx+side*7.7, cy-3+row*2.7, 1, 1.2, RUST)
    if kind == 'trike':
        trike_head(d, *head, fossil)
    elif kind == 'spino':
        sail(d, cx, cy, fossil)
        spino_head(d, *head, fossil)
    else:
        bronto_head(d, *head, fossil)
    return d.finish(True)


def generate():
    for kind in ('trike', 'spino', 'bronto'):
        for frame in range(4):
            emit('g_'+kind, dinosaur(kind, frame), frame=frame, group='ground')
        emit('g_bones_'+kind, dinosaur(kind, fossil=True), group='ground')
