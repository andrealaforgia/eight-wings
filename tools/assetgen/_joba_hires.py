"""Job A drawing primitives: logical geometry, native-texel material detail."""
from pixlib import Canvas
from _joba_materials import ACCENTS

SCALE = 2
STEEL = ('ink', 'g1', 'g2', 'g3', 'g4', 'white')
BLUE = ('ink', 'b1', 'b2', 'b3', 'b4', 'c4')


def polygon(c, points, colour):
    c.polygon([(x*SCALE, y*SCALE) for x,y in points], colour)


def panel(c, points, ramp=STEEL):
    """One-texel bevel, broad lit face, narrow shadow and reflected lower rim."""
    mask = Canvas(c.w,c.h)
    polygon(mask, points, 'white')
    dark, core, shade, mid, light, shine = ramp
    for y in range(c.h):
        xs = [x for x in range(c.w) if mask.get(x,y)[3]]
        if not xs:
            continue
        lo, hi = min(xs), max(xs)
        for x in xs:
            if not mask.get(x,y+1)[3] or not mask.get(x+1,y)[3]:
                col = core
            elif not mask.get(x,y-1)[3] or not mask.get(x-1,y)[3]:
                col = shine
            elif x >= hi-2:
                col = shade
            elif x <= lo+2:
                col = light
            else:
                col = mid if x < lo+(hi-lo)*.65 else shade
            c.set(x,y,col)
    return mask


def enamel(c, points, number):
    dark, shade, mid, light = ACCENTS[number-1]
    return panel(c,points,('ink',dark,shade,mid,light,'white'))


def rivet(c,x,y):
    c.set(x,y,'white')
    c.set(x+1,y+1,'g1')


def vent(c,x,y,w=4,n=3):
    c.rect(x-1,y-1,w+2,n*2+1,'g2')
    for i in range(n):
        c.hline(x,x+w-1,y+i*2,'ink')
        c.set(x,y+i*2+1,'g4')


def nozzle(c,x,y,w,frame):
    c.rect(x-1,y-2,w+2,5,'g1')
    c.hline(x-1,x+w,y-2,'g4')
    c.vline(x-1,y-1,y+1,'white')
    for inset,col in enumerate(('o1','o2','o3','y1','y2','white')):
        left=x+inset//2
        width=w-inset
        if width>0:
            c.rect(left,y,width,max(1,5-inset//2),col)
    c.rect(x+1,y,max(1,w-2),2,'o3' if frame else 'y2')
    c.hline(x+1,x+w-2,y,'y2' if frame else 'white')
    c.set(x+w//2,y+1,'white' if frame else 'y2')
    c.hline(x,x+w-1,y+3,'o3' if frame else 'y1')
    c.set(x+w//2,y+5,'o2' if frame else 'o3')
    c.set(x+frame,y+4,'y2' if frame else 'o2')


def reflect(c):
    """Mirror geometry exactly, then restore upper-left light on metal edges."""
    c=c.flip_h()
    source=c.copy()
    metal={Canvas._rgba(k) for k in STEEL[1:]}
    for y in range(c.h):
        for x in range(c.w):
            if source.get(x,y) not in metal:
                continue
            if source.get(x-1,y) not in metal and source.get(x,y-1) in metal:
                c.set(x,y,'g4')
            elif source.get(x+1,y) not in metal and source.get(x,y+1) in metal:
                c.set(x,y,'g1')
    return c


def nearest(c, scale):
    out=Canvas(c.w*scale,c.h*scale)
    for y in range(c.h):
        for x in range(c.w):
            if c.get(x,y)[3]:
                out.rect(x*scale,y*scale,scale,scale,c.get(x,y))
    return out
