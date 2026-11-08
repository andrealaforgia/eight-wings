"""Reads assets/spec/sprites.spec."""
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SPEC = os.path.join(ROOT, "assets", "spec", "sprites.spec")


def read_spec():
    """Returns list of dicts: name, w, h, frames, group, desc (in file order)."""
    out = []
    for line in open(SPEC):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        p = line.split(None, 5)
        out.append(dict(name=p[0], w=int(p[1]), h=int(p[2]), frames=int(p[3]), group=p[4],
                        desc=p[5] if len(p) > 5 else ""))
    return out


def files_for(entry, base):
    n = entry["frames"]
    if n == 1:
        return [os.path.join(base, entry["group"], entry["name"] + ".png")]
    return [os.path.join(base, entry["group"], "%s.%d.png" % (entry["name"], i)) for i in range(n)]
