"""Four-texel player fleet: eight interlocking swept-wing armour modules."""
from pixlib import emit
from _joba_materials import ACCENTS
from _joba_ultra import (BLUE, FIRE, canvas, poly, rect, line,
                         ellipse, panel, enamel, rivet, sphere)
from _ship5_aircraft import core, bank, modules


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
