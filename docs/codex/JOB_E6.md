# JOB E6 — new enemies and big dinosaurs
Files you own: `tools/assetgen/gen_enemies.py`, `tools/assetgen/gen_ground.py` (+ private helpers `tools/assetgen/_e6_*.py`). Groups: enemies, ground.
You may APPEND colours to `tools/assetgen/palette.py` if essential (append only, keep names, you are the only job allowed to touch it; <= 160 colours total).
Add generators for: e_weaver, e_hunter, e_gunship, e_splitter (enemies) and g_trike, g_spino, g_bronto, g_bones_trike, g_bones_spino, g_bones_bronto (ground),
exactly as described in the spec lines. Leave every existing sprite byte-identical (do not regenerate/alter them).
The three big dinosaurs must feel like creatures with character, clearly different from each other and from g_raptor/g_rex: trike = low, wide, armoured, horned;
spino = long, with a striking sail; bronto = enormous, round body, long neck, tiny head (it fills a quarter of the screen width). Each walk cycle must alternate legs
convincingly and keep a stable centre. Bones: the same pose as the live creature, bone-white, dark gaps.
