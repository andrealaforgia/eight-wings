"""Analytic native-density fire, rolled smoke, shock fronts and docking arcs."""
import math
from pixlib import emit, rng_for
from _joba_ultra import (SCALE, FIRE, canvas, poly, rect, line, ellipse, sphere)


def star(c,x,y,r,colour):
    poly(c,[(x,y-r),(x+.28,y-.28),(x+r,y),(x+.28,y+.28),
            (x,y+r),(x-.28,y+.28),(x-r,y),(x-.28,y-.28)],colour)
    line(c,x-r,y,x+r,y,colour); line(c,x,y-r,x,y+r,colour)


def ring(c,x,y,r,colour,width=.25,start=0,end=math.tau):
    """Sample the annulus at native texel centres, without blended edges."""
    for yy in range(max(0,math.floor((y-r-width)*SCALE)),min(c.h,math.ceil((y+r+width)*SCALE)+1)):
        for xx in range(max(0,math.floor((x-r-width)*SCALE)),min(c.w,math.ceil((x+r+width)*SCALE)+1)):
            dx,dy=xx/SCALE-x,yy/SCALE-y
            if abs(math.hypot(dx,dy)-r)<=width/2:
                a=math.atan2(dy,dx)%math.tau
                if start<=a<=end: c.set(xx,yy,colour)


def flash(name,frame):
    size={'muzzle':8,'spark':8,'dock_flash':24}[name]
    c=canvas(size,size); m=(size-.25)/2
    if name=='dock_flash':
        r=(4,6.5,9,10.5)[frame]
        ring(c,m,m,r,'c1',.5)
        ring(c,m,m,r-.35,'c3' if frame<3 else 'c2',.3)
        ring(c,m,m,r-.6,'c4',.25,.4+frame*.2,2.7+frame*.2)
        ring(c,m,m,r-1.2,'b3',.25,3.5,5.7)
        if frame<2:
            star(c,m,m,4-frame,'c3'); star(c,m,m,2.3-frame*.5,'white')
        for i in range(8):
            a=i*math.pi/4+frame*.1
            x=m+math.cos(a)*r; y=m+math.sin(a)*r
            star(c,x,y,1.3 if frame<3 else .7,'c3')
            star(c,x,y,.8 if frame<3 else .4,'white')
        return c
    if name=='muzzle':
        r=(2.5,3.4,1.8)[frame]
        for q,col in ((1,'o1'),(.8,'o2'),(.63,'o3'),(.42,'y1'),(.25,'y2')):
            poly(c,[(m,m-r*q),(m+.5*q,m-.7*q),(m+r*q,m-.5*q),
                    (m+.8*q,m+.3*q),(m+r*.6*q,m+r*q),(m,m+q),
                    (m-r*.7*q,m+r*.7*q),(m-q,m),(m-r*q,m-r*.5*q),
                    (m-.5*q,m-.8*q)],col)
        ellipse(c,m,m-.3,.45,.85,'white')
        return c
    if frame<2:
        star(c,m,m,2.2+frame*.9,'o1')
        star(c,m,m,1.9+frame*.8,'o3')
        star(c,m,m,.8+frame*.4,'white')
        for i in range(4):
            a=math.pi/4+i*math.pi/2
            line(c,m+math.cos(a),m+math.sin(a),
                 m+math.cos(a)*2.3,m+math.sin(a)*2.3,'y1')
    else:
        for i in range(6):
            a=math.pi/6+i*math.pi/3; d=2.2+frame*.25
            x=m+math.cos(a)*d; y=m+math.sin(a)*d
            line(c,x,y,x+math.cos(a)*.7,y+math.sin(a)*.7,'o2')
            rect(c,x,y,.5,.5,'y2' if frame==2 else 'o3')
    return c


def fire_lobe(c,x,y,r,cooling=False,cold=False):
    ramp=(('b1','c1','b2','c2','c3','c4','white','white') if cold else
          ('ink','r1','o1','r2','o2','o3','y1','y2') if cooling else FIRE)
    sphere(c,x,y,r,r*.94,ramp)
    # Rolled gas crescents interrupt spherical bands with deliberate curl contours.
    for j in range(2):
        cx=x+r*(.22-j*.35); cy=y+r*(.08+j*.24)
        rr=r*(.38-j*.1)
        ring(c,cx,cy,rr,ramp[2],.35,.15,4.6)
        ring(c,cx-.16,cy-.19,rr*.85,ramp[5],.25,3.1,5.9)
        if r>3:
            line(c,cx-rr*.7,cy-rr*.7,cx-rr*.2,cy-rr*.85,ramp[6])
            for i in range(4):
                a=3.3+i*.45
                bx=cx+math.cos(a)*rr; by=cy+math.sin(a)*rr
                ellipse(c,bx,by,r*.085,r*.055,ramp[4+i%2])


def smoke(c,x,y,r,ember=False):
    ramp=('ink','g1','g1','g2','g2','g3','g3','g4')
    sphere(c,x,y,r,r*.83,ramp)
    for ox,oy,rr in ((-.5,-.1,.45),(.1,-.5,.53),(.46,.05,.43)):
        sphere(c,x+r*ox,y+r*oy,r*rr,r*rr*.85,ramp)
    ring(c,x-r*.15,y-r*.17,r*.36,'g2',.25,.3,4.2)
    if ember:
        line(c,x-r*.15,y+r*.1,x+r*.2,y-r*.1,'r2')
        rect(c,x-r*.12,y,.35,.35,'o3')


def explosion(name,frame):
    size,total={'expl_s':(16,6),'expl_g':(16,6),'expl_l':(32,8),'ship_death':(48,8)}[name]
    c=canvas(size,size); m=(size-.25)/2; phase=frame/(total-1)
    cold=name=='ship_death'
    if frame==0:
        star(c,m,m,size*.27,'c1' if cold else 'o1')
        star(c,m,m,size*.24,'c3' if cold else 'o3')
        star(c,m,m,size*.17,'c4' if cold else 'y1')
        ellipse(c,m,m,size*.065,size*.08,'white')
        return c
    rng=rng_for(name)
    angles=[i*math.pi/4+rng.uniform(-.17,.17) for i in range(8)]
    lobes=[rng.uniform(.34,.49) for _ in range(8)]
    spread=[rng.uniform(.56,.74) for _ in range(8)]
    radius=size*(.18,.26,.32,.34,.34,.32,.29)[frame-1]
    if phase<.77:
        fire_lobe(c,m,m,radius*.94,phase>.52,cold and frame<3)
        for i in (5,6,4,7,3,0,2,1):
            a=angles[i]
            x=m+math.cos(a)*radius*spread[i]; y=m+math.sin(a)*radius*spread[i]
            fire_lobe(c,x,y,radius*lobes[i],phase>.52,cold and frame<3)
        # Tapered jets break the rim into directional tongues of burning gas.
        if phase<.6:
            for i in (0,2,5,7):
                a=angles[i]; dx,dy=math.cos(a),math.sin(a)
                side=(-dy,dx); reach=radius*1.13
                x=m+dx*radius*.8; y=m+dy*radius*.8
                tipx=m+dx*reach; tipy=m+dy*reach
                for q,col in ((1,'r2'),(.7,'o3'),(.32,'y1')):
                    poly(c,[(x+side[0]*q,y+side[1]*q),(tipx,tipy),
                            (x-side[0]*q,y-side[1]*q)],col)
        if phase<.5:
            fire_lobe(c,m-radius*.1,m-radius*.15,radius*.48,False,cold)
            ellipse(c,m-radius*.17,m-radius*.27,radius*.16,radius*.23,'white')
        else:
            smoke(c,m-radius*.32,m-radius*.3,radius*.36,True)
            fire_lobe(c,m+radius*.22,m+radius*.2,radius*.36,True)
    else:
        for i in range(5):
            a=i*math.tau/5
            x=m+math.cos(a)*size*.24
            y=m+math.sin(a)*size*.24-(frame-total+2)*.8
            smoke(c,x,y,size*(.115 if frame==total-2 else .075),frame==total-2)
    for i,a in enumerate(angles):
        dist=min(size*.43,radius+phase*size*.11)
        x=m+math.cos(a)*dist; y=m+math.sin(a)*dist
        if frame<total-1:
            length=.8+size*.035
            line(c,x,y,x-math.cos(a)*length,y-math.sin(a)*length,
                 'c3' if cold and frame<3 else 'o3' if phase<.6 else 'r2')
            # Angular fragments have shaded bodies and a single hot leading edge.
            poly(c,[(x-.45,y-.2),(x+.3,y-.55),(x+.6,y+.2),(x,y+.5)],'g1')
            line(c,x-.45,y-.2,x+.3,y-.55,'g4' if i%3==0 else 'y1')
    if cold and frame<7:
        r=(0,7,11.5,15.5,19,21.5,22,22)[frame]
        ring(c,m,m,r,'c1' if frame<5 else 'b1',.55)
        ring(c,m,m,r-.4,'c4' if frame<4 else 'c2',.3)
        for i in range(8):
            a=i*math.pi/4
            star(c,m+math.cos(a)*r,m+math.sin(a)*r,
                 .9 if frame<5 else .5,'white' if frame<4 else 'c3')
    if name=='expl_g':
        ellipse(c,8,13.7,5.6,1,'g1')
        if frame<4:
            ellipse(c,7.8,13.4,3.8,.45,'r2')
            line(c,5,13,9.5,13,'o3')
        if frame>=3: smoke(c,6.5,5.6-(frame-3)*.8,2.8,frame==3)
    return c


def cloud(frame):
    c=canvas(32,16)
    shapes=(
        ((6,9.5,5,3),(12,6.5,6,4.5),(20,8.5,7,4),(26.5,10.5,4,2)),
        ((5,10.5,4,2),(11,8.5,6,4),(18,5.5,5,3.5),(24,9.5,6,4)),
        ((7,8.5,6,4),(15,7.5,7,5),(23,10.5,6,3),(28,10.5,2,2)))
    for x,y,rx,ry in shapes[frame]:
        ellipse(c,x,y,rx,ry,'b4')
    for x,y,rx,ry in shapes[frame]:
        ellipse(c,x-.4,y-.8,rx-.5,max(.4,ry-.8),'g4')
        ellipse(c,x-1.2,y-1.1,rx*.67,ry*.56,'white')
        ellipse(c,x+rx*.4,y+ry*.1,rx*.35,ry*.28,'g4')
        line(c,x-rx*.4,y-ry*.8,x,y-ry*.85,'white')
    src=c.copy()
    for y in range(c.h):
        for x in range(c.w):
            if src.get(x,y)[3] and any(not src.get(x+dx,y+dy)[3]
                                      for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))):
                if (x+y)%2: c.set(x,y,None)
    return c


def generate():
    for name,total in [('muzzle',3),('spark',4),('expl_s',6),('expl_l',8),
                       ('expl_g',6),('ship_death',8),('dock_flash',4),('cloud',3)]:
        for frame in range(total):
            if name=='cloud': c=cloud(frame)
            elif name.startswith('expl_') or name=='ship_death': c=explosion(name,frame)
            else: c=flash(name,frame)
            emit(name,c,frame,group='fx')


if __name__=='__main__': generate()
