"""Check docking topology, frame stability and reproducibility for Job A2."""
import hashlib
import os
import sys

HERE=os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0,os.path.dirname(HERE))
from spec import read_spec, files_for, ROOT
from validate_assets import read_png
from gen_ship import core, modules, projectile
from _joba_preview import composite
from _joba_ultra import SCALE

GROUPS={'ship','fx','pickups','ui'}


def digest():
    return {path:hashlib.sha256(open(path,'rb').read()).hexdigest()
            for entry in read_spec() if entry['group'] in GROUPS
            for path in files_for(entry,os.path.join(ROOT,'assets','generated'))}


def mask(c):
    return tuple(bool(p[3]) for p in c.px)


def connected(c):
    opaque={(x,y) for y in range(c.h) for x in range(c.w) if c.get(x,y)[3]}
    queue=[next(iter(opaque))]; visited=set(queue)
    while queue:
        x,y=queue.pop()
        for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
            p=(x+dx,y+dy)
            if p in opaque and p not in visited:
                visited.add(p); queue.append(p)
    return len(visited),len(opaque)


def checks():
    for frame in range(2):
        c=composite(frame)
        reached,total=connected(c)
        assert reached==total, ('Disconnected composite pixels',frame,total-reached)
    for frame in range(2):
        pieces=modules(frame)
        for left,right in ((0,1),(4,5),(6,7)):
            assert mask(pieces[left].flip_h())==mask(pieces[right]), 'Asymmetric paired modules'
    for a,b in zip([core(0)]+modules(0),[core(1)]+modules(1)):
        assert mask(a)==mask(b), 'Hull outline jitters'
        assert a.px!=b.px, 'Hull animation has no visible change'
    for name in ('shot_f','shot_ph'):
        assert projectile(name,0).px!=projectile(name,1).px
    for entry in read_spec():
        if entry['group'] not in GROUPS: continue
        images=[read_png(p) for p in files_for(entry,os.path.join(ROOT,'assets','generated'))]
        # The validator permits some duplicate frames; here every frame must differ.
        if entry['frames']>1 and entry['name']!='font':
            assert len({b''.join(rows) for _,_,rows in images})==len(images), entry['name']
        if entry['name']=='font': continue
        w,h,rows=images[0]
        # At least one original 4x4 pixel area must contain native-detail texels.
        detail=False
        for y in range(0,h-SCALE+1,SCALE):
            for x in range(0,w-SCALE+1,SCALE):
                block={bytes(rows[yy][xx*4:xx*4+4])
                       for yy in range(y,y+SCALE) for xx in range(x,x+SCALE)}
                detail |= len(block)>1
        assert detail, 'No native texel detail: '+entry['name']
    print('PASS: connected composite; mirrored module masks; stable hulls; all animation frames distinct; native texel detail')


def main():
    checks()
    before=digest()
    from gen_ship import generate as ship
    from gen_fx import generate as fx
    from gen_ui import generate as ui
    ui(); ship(); fx()
    after=digest()
    assert before==after, 'Regeneration changed PNG bytes'
    print('PASS: %d PNGs reproduce byte-for-byte' % len(after))


if __name__=='__main__': main()
