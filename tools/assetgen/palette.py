"""The ONE master palette for every Eight Wings graphic (mid-1980s arcade feel).

Generators must use these names only. Ramps run dark -> light.
"""
TRANSPARENT = (0, 0, 0, 0)


def _h(s):
    s = s.lstrip("#")
    return (int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16), 255)


PALETTE = {name: _h(v) for name, v in {
    # neutrals / outline
    "ink": "#0b0b14", "g1": "#1c1f2e", "g2": "#3a3f55", "g3": "#6b7390", "g4": "#a4adc7", "white": "#f4f6ff",
    # reds
    "r1": "#3d0a14", "r2": "#8c1c2a", "r3": "#d8364a", "r4": "#ff7a7a",
    # oranges / yellows (fire)
    "o1": "#6b2a0a", "o2": "#c4561a", "o3": "#ff9a2e", "y1": "#ffd25a", "y2": "#fff0a0",
    # greens
    "n1": "#0e2e1a", "n2": "#1f6b34", "n3": "#3fae4a", "n4": "#8de06a",
    # cyans / teals
    "c1": "#0a2e44", "c2": "#1a6e8c", "c3": "#3ec0d8", "c4": "#9af0f4",
    # blues
    "b1": "#121a4a", "b2": "#2a3fa0", "b3": "#4f7bff", "b4": "#9bb8ff",
    # purples
    "p1": "#2a1048", "p2": "#6a2aa0", "p3": "#b05ae0", "p4": "#e0a0ff",
    # browns / sand
    "s1": "#3a2410", "s2": "#7a4e22", "s3": "#b88a4a", "s4": "#e0c080", "s5": "#f4e4b0",
    # magenta accents
    "m1": "#8c1c6a", "m2": "#e04aa0",
}.items()}

# Hard limit checked by tools/validate_assets.py across the whole atlas.
MAX_COLOURS = len(PALETTE)
