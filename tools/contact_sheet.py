#!/usr/bin/env python3
"""Render a magnified contact sheet of one sprite group for eyeballing: contact_sheet.py <group> [scale] [out.png]"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE); sys.path.insert(0, os.path.join(HERE, "assetgen"))
from spec import read_spec, files_for, ROOT
from pixlib import Canvas
import validate_assets as V

group = sys.argv[1]; scale = int(sys.argv[2]) if len(sys.argv) > 2 else 3
out = sys.argv[3] if len(sys.argv) > 3 else "/tmp/sheet_%s.png" % group
GEN = os.path.join(ROOT, "assets", "generated")
frames = []
for e in read_spec():
    if e["group"] != group: continue
    if e["name"] == "font": continue
    for f in files_for(e, GEN):
        frames.append(V.read_png(f))
maxw = 1000 // scale
x = y = rowh = 0
placements = []
for w, h, rows in frames:
    if x + w + 3 > maxw: x, y, rowh = 0, y + rowh + 3, 0
    placements.append((x, y, w, h, rows)); x += w + 3; rowh = max(rowh, h)
H = y + rowh + 3
sheet = Canvas(maxw * scale, H * scale)
sheet.rect(0, 0, sheet.w, sheet.h, "g2")
for (px, py, w, h, rows) in placements:
    for yy in range(h):
        for xx in range(w):
            r, g, b, a = rows[yy][xx * 4:xx * 4 + 4]
            if a:
                sheet.rect((px + xx) * scale, (py + yy) * scale, scale, scale, (r, g, b, 255))
sheet.save_png(out); print(out)
