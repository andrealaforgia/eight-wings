#!/usr/bin/env python3
"""Validate generated sprites (and the packed atlas, if present) against assets/spec/sprites.spec.

Checks: every file exists; PNG decodes; exact dimensions; alpha is binary; every opaque pixel is a palette colour;
sprite is not empty; animation frames are not all identical; (tiles) fully opaque; atlas metadata inside bounds,
no overlap, atlas pixels equal the source sprites. Exit status 0 only if everything passes.
Usage: validate_assets.py [--group G] [--no-atlas]
"""
import os
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "assetgen"))
from spec import read_spec, files_for, ROOT  # noqa: E402
from palette import PALETTE  # noqa: E402

GEN = os.path.join(ROOT, "assets", "generated")
PAL = set(v[:3] for v in PALETTE.values())


def read_png(path):
    d = open(path, "rb").read()
    assert d[:8] == b"\x89PNG\r\n\x1a\n", "not a PNG"
    pos, idat, w = 8, b"", None
    while pos < len(d):
        ln, tag = struct.unpack(">I4s", d[pos:pos + 8])
        body = d[pos + 8:pos + 8 + ln]
        pos += 12 + ln
        if tag == b"IHDR":
            w, h, bd, ct = struct.unpack(">IIBB", body[:10])
            assert bd == 8 and ct == 6, "must be 8-bit RGBA"
        elif tag == b"IDAT":
            idat += body
    raw = zlib.decompress(idat)
    stride = w * 4
    rows, prev = [], bytearray(stride)
    for y in range(h):
        f = raw[y * (stride + 1)]
        line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = line[i - 4] if i >= 4 else 0
            b = prev[i]
            c = prev[i - 4] if i >= 4 else 0
            if f == 1: line[i] = (line[i] + a) & 255
            elif f == 2: line[i] = (line[i] + b) & 255
            elif f == 3: line[i] = (line[i] + ((a + b) >> 1)) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 255
        rows.append(line)
        prev = line
    return w, h, rows


def check_sprite(path, e, errors):
    try:
        w, h, rows = read_png(path)
    except Exception as ex:  # noqa
        errors.append("%s: unreadable (%s)" % (path, ex)); return None
    if (w, h) != (e["w"], e["h"]):
        errors.append("%s: size %dx%d, spec %dx%d" % (os.path.basename(path), w, h, e["w"], e["h"])); return None
    opaque = 0
    for y in range(h):
        for x in range(w):
            r, g, b, a = rows[y][x * 4:x * 4 + 4]
            if a not in (0, 255):
                errors.append("%s: semi-transparent pixel at %d,%d" % (os.path.basename(path), x, y)); return None
            if a:
                opaque += 1
                if (r, g, b) not in PAL:
                    errors.append("%s: off-palette colour %02x%02x%02x at %d,%d" % (os.path.basename(path), r, g, b, x, y)); return None
            elif e["group"] == "tiles":
                errors.append("%s: tile has transparent pixel at %d,%d" % (os.path.basename(path), x, y)); return None
    if opaque == 0 and os.path.basename(path) != "font.0.png":  # the space glyph is legitimately blank
        errors.append("%s: empty sprite" % os.path.basename(path))
    return bytes(b for r in rows for b in r)


def main():
    group = None
    if "--group" in sys.argv:
        group = sys.argv[sys.argv.index("--group") + 1]
    errors, count = [], 0
    for e in read_spec():
        if group and e["group"] != group:
            continue
        frames = []
        for f in files_for(e, GEN):
            if not os.path.exists(f):
                errors.append("missing: " + os.path.relpath(f, ROOT)); continue
            data = check_sprite(f, e, errors)
            count += 1
            frames.append(data)
        good = [f for f in frames if f]
        if e["frames"] > 1 and e["name"] != "font" and len(good) == e["frames"] and len(set(good)) == 1:
            errors.append("%s: all animation frames identical" % e["name"])
    if "--no-atlas" not in sys.argv and not group:
        try:
            import pack_atlas
            errors += pack_atlas.verify()
        except ImportError:
            pass
    for e in errors[:80]:
        print("FAIL", e)
    print("checked %d sprite files, %d problems" % (count, len(errors)))
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
