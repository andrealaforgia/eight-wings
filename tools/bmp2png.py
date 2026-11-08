#!/usr/bin/env python3
"""Converts the engine's BMP frame captures (24/32-bit, uncompressed) to PNG. Stdlib only."""
import struct
import sys
import zlib


def convert(src, dst):
    d = open(src, "rb").read()
    off, = struct.unpack_from("<I", d, 10)
    w, h, _, bpp = struct.unpack_from("<iiHH", d, 18)
    flip = h > 0
    h = abs(h)
    bytespp = bpp // 8
    stride = (w * bytespp + 3) & ~3
    rows = []
    for y in range(h):
        sy = h - 1 - y if flip else y
        row = d[off + sy * stride: off + sy * stride + w * bytespp]
        out = bytearray()
        for x in range(w):
            b, g, r = row[x * bytespp], row[x * bytespp + 1], row[x * bytespp + 2]
            out += bytes((r, g, b))
        rows.append(b"\x00" + bytes(out))

    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        return c + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) \
        + chunk(b"IDAT", zlib.compress(b"".join(rows), 9)) + chunk(b"IEND", b"")
    open(dst, "wb").write(png)


if __name__ == "__main__":
    for s in sys.argv[1:]:
        convert(s, s[:-4] + ".png")
