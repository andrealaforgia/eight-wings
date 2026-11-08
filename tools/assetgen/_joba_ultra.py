"""Native four-texel geometry and discrete, palette-only material lighting."""
import math
from pixlib import Canvas
from _joba_materials import ACCENTS

SCALE = 4
STEEL = ('ink', 'g1', 'b1', 'g2', 'g3', 'g4', 'b4', 'white')
BLUE = ('ink', 'b1', 'c1', 'b2', 'c2', 'b3', 'b4', 'c4')
FIRE = ('r1', 'r2', 'o1', 'o2', 'o3', 'y1', 'y2', 'white')


def canvas(w, h):
    return Canvas(round(w*SCALE), round(h*SCALE))


def poly(c, points, colour):
    c.polygon([(x*SCALE, y*SCALE) for x, y in points], colour)


def rect(c, x, y, w, h, colour):
    c.rect(round(x*SCALE), round(y*SCALE), round(w*SCALE), round(h*SCALE), colour)


def line(c, x0, y0, x1, y1, colour):
    c.line(round(x0*SCALE), round(y0*SCALE), round(x1*SCALE), round(y1*SCALE), colour)


def ellipse(c, x, y, rx, ry, colour):
    c.ellipse(x*SCALE, y*SCALE, rx*SCALE, ry*SCALE, colour)


def panel(c, points, ramp=STEEL):
    """Rasterise a bevelled polygon with thin edges and a broad faceted face."""
    mask=Canvas(c.w, c.h)
    poly(mask, points, 'white')
    for y in range(c.h):
        xs=[x for x in range(c.w) if mask.get(x,y)[3]]
        if not xs:
            continue
        lo, hi=xs[0], xs[-1]
        for x in xs:
            if not mask.get(x+1,y)[3] or not mask.get(x,y+1)[3]:
                k=0
            elif not mask.get(x-1,y)[3] or not mask.get(x,y-1)[3]:
                k=7
            elif not mask.get(x-2,y)[3] or not mask.get(x,y-2)[3]:
                k=6
            elif not mask.get(x+2,y)[3] or not mask.get(x,y+2)[3]:
                k=2
            else:
                u=(x-lo)/max(1, hi-lo)
                k=5 if u<.19 else 4 if u<.64 else 3
            c.set(x,y,ramp[k])
    return mask


def enamel(c, points, number):
    dark, shade, mid, light=ACCENTS[number-1]
    return panel(c, points, ('ink',dark,dark,shade,mid,mid,light,'white'))


def rivet(c, x, y):
    x,y=round(x*SCALE),round(y*SCALE)
    c.rect(x-1,y-1,3,3,'g1')
    c.set(x-1,y-1,'g4'); c.set(x,y-1,'white'); c.set(x,y,'g3')


def vent(c, x, y, w=2, n=4):
    rect(c,x-.25,y-.25,w+.5,n*.65+.25,'g2')
    for i in range(n):
        rect(c,x,y+i*.65,w,.3,'ink')
        line(c,x,y+i*.65+.3,x+w-.25,y+i*.65+.3,'g4')


def barrel(c, x, y, length=6, width=1.5):
    panel(c,[(x,y+.5),(x+.3,y),(x+width-.3,y),(x+width,y+.5),
             (x+width,y+length),(x,y+length)])
    rect(c,x+.35,y+.5,width-.7,.55,'ink')
    line(c,x+.35,y+1.4,x+.35,y+length-.4,'white')
    for offset in (2, length-1.25):
        line(c,x,y+offset,x+width-.25,y+offset,'g1')
        line(c,x+.3,y+offset+.25,x+width-.5,y+offset+.25,'g4')


def nozzle(c, x, y, w, frame):
    panel(c,[(x-.3,y-1.2),(x+w+.3,y-1.2),(x+w,y+1),(x,y+1)])
    for i in range(round(w*2)):
        line(c,x+i*.5,y-.9,x+i*.5,y-.1,'g1')
    # Fixed outer envelope; the travelling hot tongues animate inside it.
    points=[(x,y),(x+w,y),(x+w-.25,y+1.8),(x+w*.7,y+2.5),
            (x+w*.5,y+3.1),(x+w*.25,y+2.5),(x+.25,y+1.8)]
    poly(c,points,'r1')
    for inset,col in enumerate(FIRE[2:]):
        q=inset*.12
        poly(c,[(x+.25+q,y+.15),(x+w-.25-q,y+.15),
                (x+w*.65,y+2.2-q),(x+w*.5,y+2.7-q*2),
                (x+w*.32,y+2.1-q)],col)
    ellipse(c,x+w*(.42 if frame else .58),y+1,.18,.7,'o3' if frame else 'white')
    line(c,x+.4,y+.1,x+w-.4,y+.1,'white' if frame else 'y1')


def sphere(c,x,y,rx,ry,ramp=FIRE):
    """Analytic elliptical surface, quantised to eight solid palette steps."""
    for yy in range(max(0,math.floor((y-ry)*SCALE)),min(c.h,math.ceil((y+ry)*SCALE)+1)):
        for xx in range(max(0,math.floor((x-rx)*SCALE)),min(c.w,math.ceil((x+rx)*SCALE)+1)):
            dx=(xx/SCALE-x)/rx; dy=(yy/SCALE-y)/ry
            d=dx*dx+dy*dy
            if d>1:
                continue
            z=math.sqrt(1-d)
            light=-.42*dx-.5*dy+.75*z
            k=max(0,min(7,int((light+.05)*7)))
            if d>.96:
                k=1 if dx+dy>0 else 4
            if d>.86 and dx>.25 and dy>.25:
                k=3
            c.set(xx,yy,ramp[k])


def reflect(c):
    """Mirror the mask; restore left-facing metal bevels after reflection."""
    out=c.flip_h(); src=out.copy()
    metal={Canvas._rgba(v) for v in STEEL}
    for y in range(out.h):
        for x in range(out.w):
            if src.get(x,y) not in metal or src.get(x,y)==Canvas._rgba('ink'):
                continue
            if src.get(x-1,y)==Canvas._rgba('ink'):
                out.set(x,y,'g4')
            elif src.get(x+1,y)==Canvas._rgba('ink'):
                out.set(x,y,'g1')
    return out


def nearest(c, scale):
    out=Canvas(c.w*scale,c.h*scale)
    for y in range(c.h):
        for x in range(c.w):
            out.rect(x*scale,y*scale,scale,scale,c.get(x,y))
    return out


def downsample(c, factor=SCALE):
    out=Canvas(c.w//factor,c.h//factor)
    for y in range(out.h):
        for x in range(out.w):
            out.set(x,y,c.get(x*factor+factor//2,y*factor+factor//2))
    return out
