"""Render the native fleet and its logical-size silhouette, only into /tmp."""
import os
import sys
from pixlib import Canvas
from gen_ship import core, modules
from gen_ui import lettering
from _joba_ultra import SCALE, canvas, nearest, downsample

OFFSETS=((-12,2),(12,2),(0,-16),(0,15),(-22,6),(22,6),(-10,-12),(10,-12))


def composite(frame=0):
    # The contract's offsets require 56px across, exceeding its 48px preview.
    out=canvas(64,64)
    centre=out.w//2
    body=core(frame)
    out.blit(body,centre-body.w//2,centre-body.h//2)
    for module,(x,y) in zip(modules(frame),OFFSETS):
        ox=centre+SCALE*x-module.w//2; oy=centre+SCALE*y-module.h//2
        overlap=sum(bool(module.get(xx,yy)[3] and out.get(ox+xx,oy+yy)[3])
                    for yy in range(module.h) for xx in range(module.w))
        assert overlap, 'Module at %s has no docking overlap' % ((x,y),)
        out.blit(module,ox,oy)
    return out


def logical_sheet(group):
    sys.path.insert(0,os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    from spec import read_spec, files_for, ROOT
    from validate_assets import read_png
    thumbs=[]
    for entry in read_spec():
        if entry['group']!=group or entry['name']=='font': continue
        for path in files_for(entry,os.path.join(ROOT,'assets','generated')):
            w,h,rows=read_png(path)
            thumb=Canvas(w//SCALE,h//SCALE)
            for y in range(thumb.h):
                for x in range(thumb.w):
                    offset=(x*SCALE+SCALE//2)*4
                    thumb.set(x,y,rows[y*SCALE+SCALE//2][offset:offset+4])
            thumbs.append(thumb)
    x=y=rowh=0; placements=[]
    for thumb in thumbs:
        if x+thumb.w+2>256:
            x=0; y+=rowh+3; rowh=0
        placements.append((thumb,x,y)); x+=thumb.w+3; rowh=max(rowh,thumb.h)
    sheet=Canvas(256,y+rowh+2); sheet.rect(0,0,sheet.w,sheet.h,'g2')
    for thumb,x,y in placements: sheet.blit(thumb,x,y)
    nearest(sheet,4).save_png('/tmp/joba_'+group+'_logical.png')


def main():
    c=composite(); composite(1)
    background=canvas(64,64); background.rect(0,0,background.w,background.h,'g2')
    background.blit(c,0,0)
    nearest(background,4).save_png('/tmp/joba_composite.png')
    nearest(downsample(background),8).save_png('/tmp/joba_logical.png')
    # Exact native 48x64 contract crop, available alongside the uncropped fleet.
    crop=canvas(48,64); crop.blit(background,-8*SCALE,0)
    nearest(downsample(crop),8).save_png('/tmp/joba_composite_48x64.png')
    glyphs=Canvas(512,192); glyphs.rect(0,0,512,192,'g2')
    for code in range(32,127):
        index=code-32
        lettering(glyphs,chr(code),(index%16)*32+4,(index//16)*32+2)
    nearest(glyphs,2).save_png('/tmp/joba_font.png')
    nearest(downsample(glyphs),6).save_png('/tmp/joba_font_logical.png')
    for group in ('ship','fx','pickups','ui'): logical_sheet(group)
    print('/tmp/joba_composite.png /tmp/joba_logical.png /tmp/joba_font.png; four logical contact sheets')


if __name__=='__main__': main()
