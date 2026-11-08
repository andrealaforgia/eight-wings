"""Four-texel player fleet: eight interlocking swept-wing armour modules."""
from pixlib import emit
from _joba_materials import ACCENTS
from _joba_ultra import (BLUE, FIRE, canvas, poly, rect, line,
                         ellipse, panel, enamel, rivet, vent, barrel, nozzle,
                         sphere, reflect)


def core(frame=0, lean=0):
    c=canvas(16,24)
    # Only the half-mask is authored; both docking shoulders are exact mirrors.
    poly(c,[(8,.3),(6.3,2.3),(5.6,7),(1,4.1),(.25,4.8),(.25,6),
            (4.9,9),(4.8,11.8),(.25,15.2),(.25,17.6),(5,17.4),
            (5.8,20.8),(8,21.5)],'g3')
    c.mirror_left_to_right()
    panel(c,[(7.9,.25),(6,2.8),(5.3,10.3),(5.5,16.5),(6,20.8),
             (10,20.8),(10.5,16.5),(10.7,10.3),(10,2.8),(8.1,.25)],BLUE)
    for right in (False,True):
        def pts(p):
            return [(16-x,y) for x,y in p] if right else p
        panel(c,pts([(.2,4.4),(1.2,3.5),(6.2,4.8),(6,7.4),(.2,6.2)]))
        panel(c,pts([(.2,15.2),(4.8,11.7),(6.3,12.7),(5.7,17.1),(.2,18)]))
        panel(c,pts([(.8,15.4),(4.7,12.6),(5.2,13.2),(4.5,16.3),(1,17.1)]),BLUE)
        x=10.8 if right else 3.25
        vent(c,x,14.7,1.7,3)
        rivet(c,13.7 if right else 2,16.6)
        line(c,10.6 if right else 5.3,5.2,13.9 if right else 2.1,5.6,'g1')
        line(c,10.7 if right else 5.2,5.5,13.8 if right else 2.2,5.9,'c3')
    # Raised nose keel and canopy use curved glass rather than a flat colour patch.
    panel(c,[(7.7,.4),(6.9,2.3),(6.5,5.2),(7.1,7.2),(8.9,7.2),
             (9.5,5.2),(9.1,2.3),(8.3,.4)])
    ellipse(c,8+lean*.25,9.8,2.25,4.8,'ink')
    sphere(c,8+lean*.25,9.5,1.95,4.2,('ink','b1','c1','b2','c2','c3','c4','white'))
    poly(c,[(6.25,10.9),(9.7,8.2),(9.8,10.5),(6.6,13)],'c2')
    line(c,6.9,6.6,6.55,9,'white'); line(c,6.8,9.7,6.9,10.4,'c4')
    ellipse(c,7.3,6.35,.3,.4,'white')
    line(c,7.3,13.8,8.5,14.4,'g1')
    panel(c,[(7,14.3),(9,14.3),(9.4,18.9),(8.8,20.4),(7.1,20.4),(6.6,18.9)])
    line(c,7.4,14.8,7.4,18.5,'white'); line(c,8.3,15.2,8.3,18.8,'c2')
    for y in (17.2,18,18.8): line(c,7.5,y,8.8,y,'g1')
    nozzle(c,6.5,20.5,3,frame)
    rect(c,7.1,.25,1.8,.7,'g3'); line(c,7.1,.25,8.7,.25,'white')
    c.outline('ink')
    if lean:
        # Banking is a new projection at native density, retaining the anchor.
        source=c; c=canvas(16,24)
        for y in range(c.h):
            for x in range(c.w):
                sx=round((c.w-1)/2+(x-(c.w-1)/2)/.79+lean*(y-48)*.045)
                c.set(x,y,source.get(sx,y))
    return c


def bank(left=True):
    return core(lean=-1 if left else 1)


def wing(frame,right=False):
    c=canvas(12,16)
    number=2 if right else 1
    panel(c,[(2.5,.4),(4.7,.4),(5.6,4.2),(11.9,6.2),(11.9,13.7),
             (6.3,13.1),(4.5,14.6),(.4,14.6),(.4,8.6),(2.5,5.4)])
    enamel(c,[(.8,8.6),(4.4,5.5),(6.2,6.1),(11.9,8),
              (11.9,11.3),(6,10.5),(3.9,12.7),(.8,12.7)],number)
    panel(c,[(1.8,4.6),(4.9,4.6),(5.5,11.5),(4.4,14),(1.6,14)])
    barrel(c,2.7,.25,8,1.5)
    vent(c,2.3,10,2,4)
    panel(c,[(5.8,11.2),(11.9,11.9),(11.9,13.7),(5.8,13)])
    line(c,6.2,12,11.5,12.7,'g1')
    for x,y in ((1,9),(6.3,7),(10.7,8.5),(7.3,12.1)):
        rivet(c,x,y)
    for i in range(3):
        line(c,6.6+i*.7,8,7+i*.7,8.5,ACCENTS[number-1][3])
    rect(c,6.3,9.3,2,.25,'g1')
    rect(c,6.3+frame*1.2,9.3,.7,.25,'white')
    nozzle(c,2,12.7,2.6,frame)
    c.outline('ink')
    return reflect(c) if right else c


def outer(frame,right=False):
    c=canvas(12,14)
    number=6 if right else 5
    panel(c,[(2,.25),(4.3,.25),(4.7,4.8),(11.9,6),(11.9,11.4),
             (7.2,11),(4.4,12.7),(.4,12.7),(.4,7.4),(2,4.9)])
    enamel(c,[(.8,7.8),(4.5,5),(11.9,6.6),(11.9,9),
              (5.4,8),(3.3,11.5),(.8,11.5)],number)
    panel(c,[(1.6,4.5),(4.5,4.5),(5,10.5),(4.2,12),(1.5,12)])
    barrel(c,2.4,.2,8.4,1.6)
    vent(c,2,9,2,3)
    line(c,5.6,10,11.7,10.7,'g1')
    line(c,6,10.4,11.7,11.1,'g4')
    for x,y in ((1,8),(6.6,6.9),(10.7,7.6)):
        rivet(c,x,y)
    rect(c,6.1,7.8,2.2,.4,'g1'); rect(c,6.3+frame,7.8,.7,.4,'c4')
    nozzle(c,1.8,10.7,2.6,frame)
    c.outline('ink')
    return reflect(c) if right else c


def canard(frame,right=False):
    c=canvas(8,10)
    number=8 if right else 7
    panel(c,[(1.1,.4),(3.6,.4),(4.2,3.3),(7.9,5),(7.9,9.9),
             (5.5,9.9),(4.6,7.3),(.4,5.6),(.4,2)])
    enamel(c,[(.8,2.1),(3.6,2.8),(4.1,4.7),(7.8,6),
              (7.8,8.1),(4.6,6.7),(.8,4.9)],number)
    for x in (1.2,3): barrel(c,x,.2,4.3,1)
    vent(c,1.8,5.2,1.8,2)
    panel(c,[(5.5,7.9),(7.9,7.9),(7.9,9.9),(5.5,9.9)])
    line(c,5.8,8.8,7.6,8.8,'g1')
    rect(c,5.9+frame*.75,7.5,.7,.3,'c4')
    rivet(c,1.4,4.8); rivet(c,6.8,6.6)
    c.outline('ink')
    return reflect(c) if right else c


def nose(frame):
    c=canvas(12,12)
    poly(c,[(6,.25),(4.4,2),(4,4),(1.3,6.2),(.7,8),(2.5,10.8),
            (4.5,11.9),(6,11.9)],'g3'); c.mirror_left_to_right()
    panel(c,[(6,.25),(4.4,2),(4,4),(1.3,6.2),(.7,8),(2.5,10.8),
             (4.5,11.9),(7.5,11.9),(9.5,10.8),(11.3,8),(10.7,6.2),
             (8,4),(7.6,2)])
    enamel(c,[(2.1,6.6),(4.3,4.7),(7.7,4.7),(9.9,6.6),
              (9.3,9.7),(7.6,10.8),(4.4,10.8),(2.7,9.7)],3)
    panel(c,[(4.8,.1),(7.2,.1),(7.7,8.5),(6.5,10),(5.5,10),(4.3,8.5)])
    barrel(c,5.3,.15,7.3,1.4)
    for x in (2.6,7.8): vent(c,x,7,1.5,3)
    for x in (2,10): rivet(c,x,6.5)
    line(c,4.6,10.9,7.4,10.9,'n1')
    rect(c,4.8+frame*1.5,10.9,1,.4,'n4')
    c.outline('ink')
    return c


def rear(frame):
    c=canvas(16,10)
    poly(c,[(.4,.6),(4,2.4),(5.5,.3),(8,.3),(8,7),(2,7)],'g3')
    c.mirror_left_to_right()
    panel(c,[(.4,.6),(4,2.4),(5.5,.3),(10.5,.3),(12,2.4),(15.6,.6),
             (15,6.9),(11.7,7.2),(8,5.3),(4.3,7.2),(1,6.9)])
    enamel(c,[(1.3,2),(4,3.8),(6,1.4),(10,1.4),(12,3.8),(14.7,2),
              (14.3,5.2),(11.5,5.4),(8,3.9),(4.5,5.4),(1.7,5.2)],4)
    for x in (2.2,11.2):
        vent(c,x,3.9,2.2,3); nozzle(c,x-.1,6.5,2.8,frame)
        rivet(c,x-.5,2.9)
    panel(c,[(5.7,0),(10.3,0),(10.8,3.2),(9.5,4.7),(6.5,4.7),(5.2,3.2)])
    line(c,6.4,.4,6.1,2.5,'white'); line(c,9.4,.4,9.8,2.5,'g1')
    line(c,7.2,.7,7.2,2.5,'c4')
    rect(c,6.5+frame*1.7,4.7,1,.4,'p4')
    c.outline('ink')
    return c


def phoenix(frame):
    c=canvas(32,32)
    lift=(0,2.2,5,2.8)[frame]
    # Scalloped individual flight feathers follow one shared shoulder spar.
    for i in range(7):
        root=(15-i*.48,12+i*.65)
        tip=(1.1+i*1.65,3+i*2.15+lift*(1-i*.07))
        poly(c,[root,(tip[0]-.7,tip[1]),(tip[0],tip[1]+4.6),
                (tip[0]+1,tip[1]+3.1),(15.8,22)],'r1')
        poly(c,[(root[0],root[1]+.3),(tip[0],tip[1]+.6),
                (tip[0]+.5,tip[1]+3.7),(15.5,20.7)],'o2')
        poly(c,[root,(tip[0]+.3,tip[1]+1.2),(tip[0]+.7,tip[1]+2.8),(15,19.5)],'o3')
        line(c,root[0],root[1],tip[0]+.4,tip[1]+2.6,'y2')
        for j in range(4):
            t=.2+j*.17
            x=root[0]*(1-t)+(tip[0]+.4)*t
            y=root[1]*(1-t)+(tip[1]+2.6)*t
            line(c,x,y,x-.7,y+.8,'y1')
    poly(c,[(16,8),(12,9),(1,1.5+lift),(3.5,6+lift),(12,15),(16,18)],'r2')
    poly(c,[(16,9),(12,10),(1.8,2.7+lift),(4.3,6.4+lift),(12.3,14),(16,17)],'o3')
    line(c,2.2,3.2+lift,12.7,11,'y2')
    line(c,4.3,6.1+lift,13,13,'y1')
    # Five streamers retain a stable head and tail centre while the wings beat.
    for i in range(3):
        poly(c,[(15.5-i*.6,18),(11+i*1.1,30-i*1.2),
                (14+i*.4,27-i*.7),(16,31.4),(16,18)],'r2')
        line(c,15-i*.45,20,12.2+i*1.2,28.5-i*.7,'o3')
        line(c,15.4-i*.35,20.4,13+i,27.5-i*.7,'y1')
    sphere(c,15.5,15,2.5,7.5,FIRE)
    poly(c,[(16,1),(14,3),(14.4,5),(13.8,7),(15,11),(16,11)],'o3')
    poly(c,[(16,2),(14.8,3.4),(15.3,5.2),(14.7,7),(16,10)],'y2')
    line(c,14.6,5.5,13.8,5,'y1')
    rect(c,14.7,3.9,.65,.45,'r1'); rect(c,14.8,3.8,.25,.25,'white')
    for j in range(3):
        poly(c,[(15.7-j*.5,19),(12.2+j*1.05,29.2-j),
                (14.3+j*.3,26-j*.3),(15.9,30.3),(16,19)],'o2')
        poly(c,[(15.7-j*.4,20),(13.1+j*.85,27.9-j*.6),
                (15,25.8),(16,29.2),(16,20)],'o3')
        line(c,15.5-j*.35,21,13.7+j*.75,27.4-j*.7,'y2')
    line(c,15.3,10.5,14.6,16.5,'white')
    for i in range(8):
        y=10+i*1.1
        line(c,14.3,y,15.1,y+.7,'o2')
    c.mirror_left_to_right()
    c.outline('r1')
    return c


def projectile(name,frame=0):
    w,h={'shot_p':(4,10),'shot_w':(6,10),'shot_f':(6,12),'shot_ph':(16,24)}[name]
    c=canvas(w,h)
    if name=='shot_ph':
        poly(c,[(8,.3),(4.6,4.7),(1.1,10.3),(2.2,17),(4.1,15.2),
                (6,22.8),(8,19.2),(11.5,23.3),(14.9,12.8),(12.5,5.7)],'r1')
        for inset,col in enumerate(FIRE[1:]):
            q=inset*.55
            poly(c,[(8,1.2+q),(4.5+q*.4,6),(2.1+q,10.5),
                    (3+q,15.5-q),(5.5,14-q),(6.7,20.5-q),
                    (8.3,16.5-q),(10.8,20.5-q),(13.7-q,11.6),(11,6)],col)
        for i in range(4):
            x=4+i*2.4
            line(c,x,13,x+.7*(1 if frame else -1),17+i%2*3,'o3' if i%2 else 'y1')
        ellipse(c,7.4,9,1.1,5.2,'y2')
        ellipse(c,7,7.6,.7,3.8,'white')
        from gen_fx import ring
        ring(c,10.3,12.6,1.5,'o3',.3,.3,4.9)
        ring(c,10.1,12.3,1.1,'y1',.25,3,5.7)
        for j in range(3):
            line(c,4+j*3.1,18+j%2,4.2+j*3.1,19.5+j%2,'o3')
        return c
    ramp=('ink','p1','m1','p2','m2','p3','p4','white') if name=='shot_f' else BLUE
    sphere(c,w/2-.125,h/2-.125,w*.38,h*.46,ramp)
    line(c,w/2-.5,1.3,w/2-.5,h-2,'white')
    if name=='shot_w':
        line(c,.8,3,.8,7.4,'c4'); line(c,w-1,3,w-1,7.4,'c3')
    if name=='shot_f':
        ellipse(c,w/2,3+frame*5,1.3,.45,'white')
        line(c,1,2+frame*4,1,4+frame*4,'p4')
    c.outline('ink')
    return c


def capsule(number,frame):
    from gen_ui import lettering
    c=canvas(16,16)
    panel(c,[(4,.7),(12,.7),(15.3,4),(15.3,12),(12,15.3),
             (4,15.3),(.7,12),(.7,4)])
    enamel(c,[(4.1,1.5),(11.9,1.5),(14.5,4.1),(14.5,11.9),
              (11.9,14.5),(4.1,14.5),(1.5,11.9),(1.5,4.1)],number)
    panel(c,[(4,3),(12,3),(13,4),(13,12),(12,13),(4,13),(3,12),(3,4)],
          ('ink','ink','g1','g1','g2','g2','g3','g4'))
    lettering(c,str(number),20,18)
    for x in (1.2,13.8):
        panel(c,[(x,5),(x+1,5),(x+1,11),(x,11)])
        for y in (5.5,9.8): rivet(c,x+.4,y)
    light=ACCENTS[number-1][3]
    for x,y in ((4,2),(11.5,13.7)):
        rect(c,x,y,1,.5,'white' if frame else light)
    line(c,5,1.9,10.7,1.9,'white' if frame else light)
    line(c,5,14,10.7,14,light if frame else ACCENTS[number-1][1])
    c.outline('ink')
    return c


def medal(frame):
    c=canvas(8,8)
    sphere(c,3.8,3.8,3.5,3.7,('ink','s2','o1','s3','o3','y1','y2','white'))
    ellipse(c,3.8,3.8,2.4,2.7,'s2'); ellipse(c,3.6,3.6,2.2,2.5,'y1')
    poly(c,[(3.8,1.7),(4.35,3),(5.8,3.1),(4.7,4.1),(5,5.8),
            (3.7,4.8),(2.3,5.8),(2.6,4.1),(1.6,3.1),(3.1,3)],'s3')
    line(c,3.8,1.7,3.1,3,'white'); line(c,1.6,3.1,2.6,4.1,'y2')
    x,y=(1.5,2) if frame==0 else (6,5.5)
    line(c,x-.7,y,x+.7,y,'white'); line(c,x,y-.7,x,y+.7,'white')
    c.outline('ink')
    return c


def modules(frame=0):
    return [wing(frame),wing(frame,True),nose(frame),rear(frame),
            outer(frame),outer(frame,True),canard(frame),canard(frame,True)]


def generate():
    for f in range(2):
        emit('ship_core',core(f),f,group='ship')
        for n,c in enumerate(modules(f),1): emit('ship_c'+str(n),c,f,group='ship')
        for name in ('shot_f','shot_ph'): emit(name,projectile(name,f),f,group='ship')
        for n in range(1,9): emit('pod_'+str(n),capsule(n,f),f,group='pickups')
        emit('medal',medal(f),f,group='pickups')
    emit('ship_core_bank_l',bank(),group='ship')
    emit('ship_core_bank_r',bank(False),group='ship')
    for f in range(4): emit('phoenix',phoenix(f),f,group='ship')
    for name in ('shot_p','shot_w'): emit(name,projectile(name),group='ship')


if __name__=='__main__': generate()
