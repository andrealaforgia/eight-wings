"""Four-texel terrain, drawn from geometry rather than enlarged bitmaps.

Decoration is inset from a shared perimeter. Liquid profiles depend only on
edge position; their endpoint slopes and periodic water field close exactly.
"""
import math

from pixlib import Canvas, emit, rng_for

SCALE = 4
SIZE = 16 * SCALE
LAST = SIZE - 1
GROUND = ('s3', 'n2', 'g1')
SHORE = tuple(round(3 * math.sin(math.pi * i / LAST) ** 2
                    + 1.5 * math.sin(3 * math.pi * i / LAST) ** 2)
              for i in range(SIZE))


def texel(value):
    return round(value * SCALE)


class Drawing:
    """Logical coordinates with native one-texel lines and unscaled curves."""
    def __init__(self, canvas):
        self.c = canvas

    def dot(self, x, y, colour):
        self.c.set(texel(x), texel(y), colour)

    def rect(self, x, y, w, h, colour):
        self.c.rect(texel(x), texel(y), texel(w), texel(h), colour)

    def ellipse(self, x, y, rx, ry, colour):
        self.c.ellipse(x * SCALE, y * SCALE, rx * SCALE, ry * SCALE, colour)

    def poly(self, points, colour):
        self.c.polygon([(x * SCALE, y * SCALE) for x, y in points], colour)

    def path(self, points, colour):
        points = [(texel(x), texel(y)) for x, y in points]
        for a, b in zip(points, points[1:]):
            self.c.line(*a, *b, colour)

    def arc(self, x, y, rx, ry, start, end, colour):
        points = [(x + rx * math.cos(a), y + ry * math.sin(a))
                  for a in [math.radians(start + (end - start) * i / 96)
                            for i in range(97)]]
        self.path(points, colour)


def bolt(d, x, y):
    d.ellipse(x + .12, y + .12, .38, .38, 'ink')
    d.ellipse(x, y, .28, .28, 'g3')
    d.dot(x - .1, y - .1, 'g4')
    d.path([(x, y + .1), (x + .2, y + .1)], 'g1')


def panel(d, x, y, w, h, face='g2', light='g3', shadow='ink'):
    d.rect(x + .6, y + .8, w, h, shadow)
    d.poly([(x, y + .5), (x + .5, y), (x + w - .5, y),
            (x + w, y + .5), (x + w, y + h), (x, y + h)], shadow)
    d.rect(x + .25, y + .5, w - .5, h - .75, face)
    d.path([(x + .25, y + h - .5), (x + .25, y + .5),
            (x + .75, y + .25), (x + w - .75, y + .25)], light)
    d.path([(x + w - .5, y + 1), (x + w - .5, y + h - .5),
            (x + 1, y + h - .5)], 'g1' if face.startswith('g') else 's2')


def hazard(d, x, y, w):
    for yy in range(texel(y), texel(y + .8)):
        for xx in range(texel(x), texel(x + w)):
            d.c.set(xx, yy, 's3' if (xx + yy) % 10 < 5 else 'g1')
    d.path([(x, y), (x + w, y)], 's4')


def ground(theme, name):
    c = Canvas(SIZE, SIZE)
    c.rect(0, 0, SIZE, SIZE, GROUND[theme])
    d = Drawing(c)
    variant = int(name[-1]) if '_base' in name else rng_for(name).randrange(4)
    if theme == 0:
        # Tapered crescent ridges. All curve samples are native texels.
        for row in range(3):
            start = 1.1 + (variant + row) % 3 * .55
            end = 14.8 - (variant + row * 2) % 3 * .8
            cy = 2.3 + row * 4.5 + variant * .3
            points = [(x / SCALE, cy + 1.05 * math.sin(x / 13 + row + variant * .4))
                      for x in range(texel(start), texel(end))]
            d.path(points, 's2')
            d.path([(x, y - .25) for x, y in points[5:-5]], 's4')
            d.path([(x, y + .65) for x, y in points[8:-9]], 's2')
            # A narrow ordered hatch softens the lee, never random sand dots.
            for x, y in points[7:-7]:
                if texel(x) % 3 == row % 3:
                    d.dot(x, y + .4, 's2')
        d.path([(4 + variant * .5, 9.5), (5, 9.8), (6.2, 9.7)], 's2')
        if variant == 3:
            d.path([(10.5, 6), (10, 7.2), (10.7, 8), (10.2, 9.3),
                    (11, 10.2), (10.6, 11.3)], 's2')
            d.path([(10.7, 8), (12, 7.6), (13, 8)], 's2')
            d.path([(10.5, 9.3), (11.25, 10.1)], 's4')
    elif theme == 1:
        # Low grass mats with interlocking blades, rather than isolated speckle.
        for x, y, rx in ((4.4, 4.2 + variant * .4, 2.7),
                         (11, 11.4 - variant * .3, 2.2)):
            d.ellipse(x + .2, y + .4, rx, .9, 'n1')
            d.ellipse(x, y, rx, .9, 'n2')
            for i in range(9):
                bx = x - rx + .5 + i * rx / 5
                by = y + .15 * math.sin(i * 2)
                d.path([(bx, by + .4), (bx + .3, by - .4),
                        (bx + .8, by - .65)], 'n1')
                if i % 3 == 0:
                    d.path([(bx + .15, by), (bx + .4, by - .4)], 'n3')
        d.path([(6.8, 8.6), (7.8, 8.2), (8.8, 8.3)], 'n1')
    else:
        # Nearly full-width plating: seams read as a floor, not framed buttons.
        d.poly([(.8, 1), (12.5, 1), (15, 3.5), (15, 14.5),
                (13.8, 15), (1, 15)], 'ink')
        d.poly([(1, 1.25), (12.3, 1.25), (14.7, 3.6), (14.7, 14.3),
                (13.6, 14.7), (1, 14.7)], 'g2')
        d.poly([(1.4, 2), (12, 2), (14, 4), (14, 14), (1.4, 14)], 'g1')
        d.path([(1, 12), (1, 1.5), (12.3, 1.5), (14.5, 3.7)], 'g3')
        d.path([(2, 11.5), (6.8, 11.5), (8, 12.8), (13.5, 12.8)], 'ink')
        d.path([(2, 11.75), (6.6, 11.75), (7.8, 13), (13.5, 13)], 'g2')
        for x, y in ((1.9, 2.8), (13, 3.8), (2, 13.5), (13, 13.5)):
            bolt(d, x, y)
        d.rect(10, 2.7, 2.3, .75, 'ink')
        d.rect(10.3, 2.95, 1.5, .25, 'c1')
        d.dot(10.3, 2.95, 'c2')
        if variant == 0:
            for y in (4.2, 5.1, 6, 6.9):
                d.rect(3.2, y, 3.8, .3, 'ink')
                d.path([(3.4, y + .4), (6.7, y + .4)], 'g2')
        elif variant == 1:
            d.path([(3.5, 4.5), (3.5, 8), (5, 9), (11.5, 9)], 'g2')
            for x in (6, 7, 8):
                d.rect(x, 9.6, .5, .25, 'g2')
        elif variant == 2:
            panel(d, 3, 4, 4, 4, 'g1', 'g2')
            d.path([(4, 5), (6, 5), (6, 7)], 'ink')
            d.dot(4, 5, 'c1')
        else:
            d.path([(3, 5), (6, 5), (7.5, 6.5), (12, 6.5)], 'ink')
            d.path([(3, 5.25), (5.8, 5.25), (7.3, 6.75), (12, 6.75)], 'g2')
            hazard(d, 9, 13.6, 2.5)
        # Fine directional scuffs confined to metal, not texture noise.
        for x, y in ((8, 4.5), (10.7, 10), (4, 10)):
            d.path([(x, y), (x + .8, y - .3)], 'g2')
    return c


def rock(d, x, y, radius, desert=False):
    dark, mid, light, bright = ('s1', 's2', 's3', 's4') if desert else ('g1', 'g2', 'g3', 'g4')
    r = radius
    d.ellipse(x + .6, y + .8, r + .4, r * .65, 's2' if desert else 'n1')
    d.poly([(x-r,y), (x-r*.6,y-r*.7), (x-.2,y-r), (x+r*.7,y-r*.6),
            (x+r,y), (x+r*.7,y+r*.7), (x,y+r*.8), (x-r*.7,y+r*.5)], dark)
    d.poly([(x-r+.25,y), (x-r*.6,y-r*.65), (x-.2,y-r+.25),
            (x+r*.65,y-r*.5), (x+r-.25,y-.1), (x+.3,y+r*.3), (x-r*.7,y+r*.2)], mid)
    d.poly([(x-r+.3,y-.3), (x-r*.6,y-r*.6), (x-.2,y-r+.3),
            (x+r*.4,y-r*.5), (x,y), (x-r*.6,y+.1)], light)
    d.path([(x-r*.7,y-r*.4), (x-.2,y-r+.25), (x+r*.3,y-r*.55)], bright)
    for row in range(3):
        yy = y + .4 + row * .45
        d.path([(x-r*.6, yy), (x+.1, yy+.25), (x+r*.65, yy-.35)], mid if row else light)
    d.path([(x+.5,y-r*.5), (x+.1,y-.3), (x+.6,y+.25), (x+.3,y+r*.5)], dark)
    d.path([(x+.75,y-.25), (x+1,y+.1)], light)
    d.dot(x+r*.7,y+.4,'s3' if desert else 'c1')


def tuft(d, x, y, desert=False):
    d.ellipse(x+.2, y+.4, 1.7, .5, 's2' if desert else 'n1')
    for i, (dx, dy) in enumerate(((-1.8,-1),(-1.2,-2.3),(-.2,-2.9),(.6,-2), (1.8,-2.4),(2,-.8))):
        d.path([(x,y), (x+dx*.55,y+dy*.75), (x+dx,y+dy)], 'n1')
        d.path([(x-.25,y-.15), (x+dx*.55-.25,y+dy*.75), (x+dx,y+dy)], 'n2' if desert else 'n3')
        if i % 2:
            d.dot(x+dx,y+dy,'s3' if desert else 'n2')


def detail(theme, variant, c):
    d = Drawing(c)
    if theme < 2:
        desert = theme == 0
        if variant == 0:
            rock(d, 5.7, 5.6, 3.5, desert)
            rock(d, 11.5, 11.4, 2.1, desert)
        elif variant == 1:
            tuft(d, 4.6, 6.7, desert)
            tuft(d, 11.2, 13.4, desert)
            if desert:
                for y in range(2, 9):
                    d.path([(11.1, y), (11.6, y+.25)], 's2')
                    d.path([(13.1, y+.2), (13.6, y+.4)], 's2')
        elif variant == 2 and desert:
            d.ellipse(8.2, 8.8, 5.4, 4, 's4')
            d.ellipse(7.9, 8.1, 5.3, 3.8, 's2')
            d.ellipse(8.4, 8.9, 4.3, 2.9, 's3')
            d.arc(7.8, 8, 4.8, 3.3, 185, 320, 's1')
            d.arc(8.2, 8.6, 5.4, 3.8, 20, 145, 's4')
            for rx, ry in ((3.4, 2), (2.9, 1.5)):
                d.arc(8.1, 9.1, rx, ry, 195, 285, 's2')
            for x, y, angle in ((3.3,6.8,210),(11.8,10.8,30),(9.9,4.9,290)):
                dx, dy = math.cos(math.radians(angle)), math.sin(math.radians(angle))
                d.path([(x,y), (x+dx,y+dy), (x+dx*1.3-.4,y+dy*1.4)], 's2')
        elif variant == 2:
            # Fluted shell with a shaded lip and radial ribs.
            d.ellipse(8.4, 9.8, 4.4, 2.3, 'n1')
            d.poly([(4,8.4),(4.7,6.5),(6.5,5.4),(9,5.3),(11.5,6.6),
                    (12,8.8),(9,10.8),(6.6,10.5)], 's2')
            d.ellipse(7.8, 7.7, 3.7, 2.4, 's3')
            for i in range(9):
                a = math.radians(190+i*20)
                d.path([(8,9.6), (7.8+3.5*math.cos(a),7.7+2.1*math.sin(a))], 's2')
                d.path([(8.25,9.6), (8+3.5*math.cos(a),7.7+2.1*math.sin(a))], 's4')
            d.arc(7.8,7.7,3.7,2.4,190,290,'s5')
            tuft(d, 12.5, 13)
        else:
            d.poly([(2.5,5),(6.5,2.5),(13.2,5),(12.5,10), (9,13),(3,11.5)], 's1' if desert else 'n1')
            d.poly([(2.7,5),(6.5,2.8),(13,5),(11.3,9.2),(7,11),(3.3,9.8)], 's2')
            d.poly([(3,5),(6.5,3),(12.5,5),(10,7.4),(4.5,8)], 's3')
            d.path([(3.3,5),(6.5,3.3),(12,5)], 's4')
            for i in range(4):
                d.path([(3.7+i*.15,8.6+i*.6),(7.2,9.8+i*.6), (11.7-i*.2,7.6+i*.6)], 's3' if i % 2 == 0 else 's2')
            d.path([(8.2,4.5),(7.3,6.4),(8.6,8.6),(7.6,11.5)], 's1')
            d.path([(7.5,6.5),(10.4,6),(12,6.8)], 's1')
            d.path([(8.8,8.6),(10.2,8.1)], 's4')
            if not desert:
                tuft(d, 12.3, 13.5)
    elif variant == 0:
        panel(d, 2.3, 2.4, 11, 10.5, 'g1', 'g2')
        for y in range(13, 47, 4):
            d.rect(3.5, y/4, 8, .4, 'ink')
            d.path([(3.5,y/4+.4),(11.3,y/4+.4)], 'g3')
            d.path([(3.8,y/4+.4),(11,y/4+.4)], 'g2')
        for x,y in ((2.9,3),(12.4,3),(2.9,12),(12.4,12)):
            bolt(d,x,y)
    elif variant == 1:
        panel(d,3,2.5,10,11)
        d.rect(4.1,4,7.7,5.1,'ink')
        d.rect(4.4,4.3,7.1,4.2,'c1')
        d.path([(4.6,4.6),(10.7,4.6)],'c2')
        for x in (5,6.4,7.8,9.2):
            d.rect(x,5.5,.5,2,'g2')
            d.dot(x,5.5,'g3')
        d.dot(4.6,4.6,'c3')
        d.rect(4.3,10.4,4,1,'g1')
        hazard(d,9,11,2.4)
    elif variant == 2:
        panel(d,2.4,2.5,11,10.5,'g1','g2')
        for x,y in ((3.3,3.5),(12,3.5),(3.3,11.7),(12,11.7)):
            bolt(d,x,y)
        panel(d,4.4,5,7,5,'g2','g3')
        hazard(d,4.8,7.3,6.1)
        d.path([(6.3,10.9),(9.3,10.9)],'g3')
    else:
        panel(d,3,2.5,10,11)
        panel(d,4.3,4,7.3,7.5,'g1','ink')
        d.path([(5.8,5.6),(8.6,5.6),(8.6,8),(9.7,8)],'g2')
        d.rect(6,10,3,.5,'ink')
        d.path([(6.3,10),(8.7,10)],'g3')
        for y in (5,6.3,7.6):
            d.rect(11,y,.5,.5,'g3')
    return c


def canopy(d, x, y, radius, desert=False):
    d.ellipse(x+.6,y+1,radius+.3,radius*.74,'s2' if desert else 'n1')
    d.ellipse(x,y,radius,radius*.8,'n1')
    for dx,dy,factor in ((-.38,-.15,.65),(.23,-.4,.6),(.43,.22,.49),(-.2,.3,.58)):
        cx,cy,r=x+dx*radius,y+dy*radius,radius*factor
        d.ellipse(cx,cy,r,r*.78,'n2')
        d.arc(cx,cy,r-.25,r*.7,195,280,'s3' if desert else 'n3')
        # Curving fans of leaflets share a dark midrib.
        for i in range(5):
            a=math.radians(190+i*29)
            ex,ey=cx+math.cos(a)*(r-.35),cy+math.sin(a)*(r*.65)
            d.path([(cx+.1,cy+.3),(ex,ey)],'n1')
            d.path([(cx,cy),(ex-.2,ey-.2)],'n2' if desert else 'n3')
            mx,my=(cx+ex)/2,(cy+ey)/2
            d.path([(mx,my),(mx-.45,my+.05)],'n1')
        d.arc(cx+.2,cy+.3,r-.4,r*.65,20,90,'n1')
    d.dot(x+radius*.7,y+.3,'n2')


def forest(theme, variant, c):
    d=Drawing(c)
    if theme < 2:
        positions=([(5.1,4.2,3.3),(10,6.8,3.5),(4.9,10.7,3.4),(10.8,12,2.4)] if variant==0
                   else [(10.2,4.2,3),(4.7,6.7,3.4),(9.7,10.8,3.5),(4.2,12.4,1.7)])
        if theme==0:
            rock(d,11.9,12.1,1.7,True)
            positions=positions[:3]
        for x,y,r in positions:
            canopy(d,x,y,r,theme==0)
    elif variant==0:
        panel(d,2.5,2.5,11,11,'g1','g2')
        for x in (4,7,10):
            d.rect(x,3.4,1.8,8.6,'ink')
            d.rect(x+.25,3.4,1.3,8.6,'g2')
            d.path([(x+.35,3.7),(x+.35,11.7)],'g3')
            d.path([(x+1.3,3.5),(x+1.3,11.8)],'g1')
            for y in (5,8,10.5):
                d.rect(x-.1,y,2,.6,'g1')
                d.path([(x,y),(x+1.7,y)],'g3')
                bolt(d,x+.7,y+.2)
        d.rect(4.3,12.5,1.5,.25,'c2')
    else:
        panel(d,2.3,3,7,10,'g2','g3')
        d.rect(3.5,4.6,4.7,4.7,'g1')
        d.path([(4,5),(7.5,5)],'c1')
        d.dot(4,5,'c2')
        for y in (6,7,8):
            d.path([(4,y),(7.5,y)],'ink')
        # Bent service conduit with a top-left cylindrical highlight.
        d.path([(10,4),(12.6,4),(13.3,4.8),(13.3,11.5),(12.8,12),(10,12)],'ink')
        d.path([(10,3.7),(12.5,3.7),(13,4.4),(13,11.4),(12.5,11.7),(10,11.7)],'g3')
        d.path([(10,4.2),(12.2,4.2),(12.5,4.7),(12.5,11)],'g2')
        for y in (6,9):
            d.rect(11.8,y,1.8,.6,'g1')
            d.path([(11.8,y),(13.5,y)],'g3')
        hazard(d,3.5,11.1,3.8)
    return c


def dome(d, theme):
    shadow=('s1','n1','ink')[theme]
    ramp=('s1','s2','s3','s4','s5') if theme==0 else ('ink','g1','g2','g3','g4')
    d.ellipse(8.5,8.7,5.7,5,shadow)
    # Elliptical hemisphere lit from upper left, evaluated per native texel.
    for yy in range(10,53):
        for xx in range(7,55):
            nx,ny=(xx/SCALE-7.6)/5.2,(yy/SCALE-7.3)/4.6
            q=nx*nx+ny*ny
            if q<=1:
                z=math.sqrt(1-q)
                light=max(0,min(.999,(-nx*.48-ny*.5+z*.72+.35)/1.55))
                d.c.set(xx,yy,ramp[int(light*len(ramp))])
    d.arc(7.6,7.3,5.2,4.6,190,280,ramp[-1])
    d.arc(7.6,7.3,2.5,4.6,195,350,ramp[1])
    d.arc(7.6,7.5,5,2.4,0,180,ramp[1])
    d.path([(2.5,11.4),(12.5,11.4)],shadow)
    d.path([(3,11.7),(12,11.7)],ramp[2])
    panel(d,6.2,11.5,3.4,2.2,ramp[1],ramp[2],shadow)
    d.rect(7.1,12,1.5,1.5,shadow)
    d.dot(11.8,8.8,'c1' if theme==2 else ramp[2])


def structure(theme, variant, c):
    d=Drawing(c)
    shadow=('s1','n1','ink')[theme]
    dark,mid,light,bright=('s2','s3','s4','s5') if theme==0 else ('g1','g2','g3','g4')
    if variant==0:
        panel(d,2.5,3.5,10.5,9,mid,light,shadow)
        d.poly([(3.1,4.3),(11.7,4.3),(10.7,6),(4.1,6)],light)
        d.rect(3.8,6,7.3,4.5,dark)
        # Standing-seam roof sheets and a recessed skylight.
        for x in (4,5.5,7,8.5,10,11.5):
            d.path([(x,4.6),(x,5.4)],mid)
            d.dot(x-.25,4.6,bright)
        d.rect(6.4,4.55,2.2,.65,dark)
        d.path([(6.6,4.55),(8.3,4.55)],bright)
        for x in (4.1,6.2,8.3,10.4):
            d.path([(x,6),(x,9.2)],mid)
            d.path([(x+.25,6),(x+.25,7)],bright)
        d.path([(3.1,4.2),(11.6,4.2)],bright)
        d.rect(5.7,10.3,3.5,2.1,shadow)
        d.path([(6,10.3),(8.7,10.3)],light)
        for x in (4.3,10.7):
            d.rect(x,2.5,1,1.5,dark)
            d.rect(x,2.5,1,.5,light)
        d.rect(11.5,8,.5,1,'c1' if theme==2 else light)
        for x in (3.4,4.4,10.1,11.1):
            d.rect(x,11.3,.5,.3,mid)
    elif variant==1:
        dome(d,theme)
    elif variant==2:
        for y in (4.3,10.5):
            d.ellipse(8.4,y+1.5,5.2,2.6,shadow)
            d.rect(3,y-1,9.7,2.4,dark)
            d.ellipse(7.8,y-1,4.8,2.2,mid)
            d.arc(7.8,y-1,4.5,2,190,290,light)
            d.arc(7.5,y-1.3,3.7,1.3,205,265,bright)
            d.path([(3.7,y+1.2),(11.7,y+1.2)],dark)
            for x in (5.2,10.3):
                d.path([(x,y-2.9),(x,y+1.3)],shadow)
                d.path([(x+.25,y-2.9),(x+.25,y+1.1)],light)
            d.rect(6.6,y-.7,2.5,.5,'c1' if theme==2 else light)
            for x in (6.6,7.6,8.6):
                d.dot(x,y-.7,mid)
        d.path([(12.8,4.3),(13.5,4.3),(13.5,10.5),(12.8,10.5)],dark)
        d.path([(12.8,4),(13.2,4),(13.2,10.3)],light)
    else:
        panel(d,2,2,12,12,'g2','g3',shadow)
        d.rect(3,3,10,10,'g1')
        d.path([(3.2,3.2),(12.5,3.2),(12.5,12.5)],'ink')
        for x,y in ((3.6,3.6),(11.5,3.6),(3.6,11.5),(11.5,11.5)):
            d.path([(x+.8,y),(x,y),(x,y+.8)],'g3')
        d.rect(5.8,5,1,6,'s3' if theme==2 else 'g3')
        d.rect(9.8,5,1,6,'s3' if theme==2 else 'g3')
        d.rect(6.8,7.7,3, .8,'s3' if theme==2 else 'g3')
        for x,y in ((2.5,2.5),(13,2.5),(2.5,13),(13,13)):
            bolt(d,x,y)
        if theme==2:
            hazard(d,6,12.7,4.1)
    return c


def road_pixel(theme, across, along):
    along %= LAST
    if theme==1:
        if across in (16,47):
            return 's1'
        if across in (17,18):
            return 's4'
        if across==46:
            return 's2'
        if across in (20,43):
            return 's1'
        if along % 8==1:
            return 's1'
        if along % 8==2:
            return 's3'
        if across in (22,41) and along % 8==4:
            return 's1'
        if (along+across//7)%19==7 and 23<across<40:
            return 's3'
        return 's2'
    if across==16:
        return 's2' if theme==0 else 'g3'
    if across==17:
        return 's4' if theme==0 else 'g2'
    if across>=46:
        return 's1' if theme==0 else 'ink'
    if across in (19,44):
        return 'g1'
    if across in (24,39) and along%17<12:
        return 'g1'
    if across in (30,31,32) and (8<=along<=19 or 39<=along<=50):
        return 's4' if theme==0 else 's3'
    if theme==2 and across==20 and along in (11,12,43,44):
        return 's3'
    if (across,along) in ((25,23),(26,24),(38,52),(37,53)):
        return 'g3'
    return 'g2'


def road(theme,direction,c):
    for y in range(SIZE):
        for x in range(SIZE):
            v=direction in ('v','x') and 16<=x<=47
            h=direction in ('h','x') and 16<=y<=47
            if v and h:
                colour='s2' if theme==1 else 'g2'
                if theme==1 and y%8 in (1,2):
                    colour='s1' if y%8==1 else 's3'
            elif v:
                colour=road_pixel(theme,x,y)
            elif h:
                colour=road_pixel(theme,y,x)
            else:
                continue
            c.set(x,y,colour)
    return c


def water_pixel(theme,x,y):
    x,y=x%LAST,y%LAST
    crest=(y-round(3*math.sin(x*math.tau/LAST)+math.sin(x*2*math.tau/LAST)))%LAST
    colour='b1' if theme==1 else 'c1'
    # Broad troughs below short, curling crests. Finite arcs avoid a stripe field.
    for start,end,row in ((5,43,15),(26,57,45)):
        if start<=x<=end:
            offset=crest-row
            if offset in (1,2,3):
                colour='c1' if theme==1 else 'g1'
            if offset==0:
                colour='c2'
            if offset==-1 and start+7<x<end-8:
                colour='c1' if theme==1 else 'c2'
            if offset==-1 and start+12<x<start+18:
                colour='c2' if theme<2 else 'c3'
            if offset==5 and start+8<x<end-9 and x%3==0:
                colour='c1' if theme==1 else 'g1'
    if theme==2 and crest==14 and 19<=x<=24:
        colour='c3'
    # Fine wind ripples curl away from the two large wave fronts.
    ripple = (y-round(1.7*math.sin((x-12)/11))) % LAST
    if 9<x<30 and ripple==33:
        colour='c1' if theme==1 else 'c2'
    if 37<x<52 and ripple==28:
        colour='c1' if theme==1 else 'g1'
    if 12<x<20 and ripple==32:
        colour='c2'
    return colour


def liquid(theme,mask):
    c=Canvas(SIZE,SIZE)
    for y in range(SIZE):
        for x in range(SIZE):
            colour=water_pixel(theme,x,y)
            banks={bit:12+SHORE[along]-distance for bit,distance,along in
                   ((1,y,x),(2,LAST-x,y),(4,LAST-y,x),(8,x,y)) if mask&bit}
            shore=max(banks.values(),default=-SIZE)
            # Round only adjacent banks. Opposite banks retain a clear channel;
            # the blend has compact support and cannot change shared edges.
            for a,b in ((1,2),(2,4),(4,8),(8,1)):
                if a in banks and b in banks:
                    overlap=max(0,20-abs(banks[a]-banks[b]))
                    shore=max(shore,max(banks[a],banks[b])+overlap*overlap/80)
            shore=round(shore)
            if shore>=7:
                colour=GROUND[theme]
            elif shore>=5:
                colour=('s4','s2','g2')[theme]
            elif shore>=3:
                colour=('s3','s3','g3')[theme]
            elif shore>=1:
                colour=('s2','s4','g1')[theme]
            elif shore==0:
                colour=('c2','g4','ink')[theme]
            elif shore==-1:
                colour='c2'
                if theme==1 and (x%LAST+y%LAST)%11<7:
                    colour='g4'
                if theme==2 and (x%LAST+y%LAST)%9<6:
                    colour='c3'
            elif shore==-2:
                colour='c2' if theme==1 else 'c1'
            elif shore==-3:
                colour='c1'
            c.set(x,y,colour)
    return c


def build_tiles():
    tiles={}
    for theme in range(3):
        prefix=f'tile_t{theme}_'
        for variant in range(4):
            name=prefix+f'base{variant}'
            tiles[name]=ground(theme,name)
        for kind,count,draw in [('detail',4,detail),('forest',2,forest),('struct',4,structure)]:
            for variant in range(count):
                name=prefix+f'{kind}{variant}'
                tiles[name]=draw(theme,variant,ground(theme,name))
        for direction in ('v','h','x'):
            name=prefix+'road_'+direction
            tiles[name]=road(theme,direction,ground(theme,name))
        for mask in range(16):
            tiles[prefix+f'liq{mask}']=liquid(theme,mask)
    return tiles


def generate():
    for name,canvas in build_tiles().items():
        emit(name,canvas,group='tiles')


if __name__=='__main__':
    generate()
