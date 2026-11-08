# Eight Wings — graphics job brief (from Claude Code, lead engineer, to Codex, graphics collaborator)

We are building an original, playable C11/SDL2/OpenGL vertical arcade shooter in the spirit of the 1980s classics
(vertical scrolling shooter; the player ship docks numbered modules that visibly change its shape; formation attacks;
ground bunkers; bosses). The native canvas is 256x224. YOU generate ALL of the pixel art, procedurally, with reproducible
Python scripts. No downloaded or traced art, no external image libraries (stdlib only — Pillow is NOT installed).

## Read these first
- `assets/spec/sprites.spec`  — THE CONTRACT: every sprite's name, exact size, frame count, group and description.
- `tools/assetgen/pixlib.py`  — Canvas toolkit (set/rect/line/disc/ellipse/polygon/blit/outline/flip/mirror/save_png) and `emit()`.
- `tools/assetgen/palette.py` — the single master palette. Use only these names. Tile/sprite colours must come from it.
- `tools/validate_assets.py`  — run `python3 tools/validate_assets.py --group <group>` to check your output (exit 0 = good).
- `tools/gen_assets.py`       — runs all `tools/assetgen/gen_*.py` modules; each must expose `generate()`.

## Rules
1. Write ONLY the files named in your job below (you may create small private helper modules named `tools/assetgen/_<yourjob>_*.py`).
   Do not edit pixlib.py, palette.py, the spec, the validator, or other jobs' files. If you believe the spec needs changing, say so in
   your final message instead of editing it.
2. Output via `pixlib.emit(name, canvas, frame=None|n, group="<group>")` — files land in `assets/generated/<group>/`.
3. Fully deterministic: seed from `pixlib.rng_for(name)`. Running the script twice yields byte-identical files.
4. Quality bar: mid-1980s Japanese arcade pixel art. Strong readable silhouettes at tiny sizes, 1px dark outline, 2-4 tonal steps per
   material, deliberate top-left highlights, symmetric craft for vehicles (draw left half then mirror), consistent pixel size
   (no sub-pixel scaling, no anti-aliasing, no gradients, no blurry noise). Animated frames must differ clearly but keep a stable silhouette/centre
   so that they don't jitter. Make things look designed, not noisy: hand-composed shapes (polygon/rect/line/ellipse) plus careful shading and detail.
   Avoid generic blobs. Distinct enemy types must be recognisable from each other at a glance by silhouette AND colour.
5. First get a complete, valid baseline of ALL sprites in your job passing the validator, then spend remaining effort improving quality.
6. To eyeball your work, you can render a contact sheet (write a tiny script that blits sprites enlarged 4x onto one canvas and saves
   a PNG into `/tmp`, then open it with any image viewing capability you have; if you cannot view images, rely on rigorous reasoning and
   ASCII dumps of the pixels). Do not commit contact sheets to the repo.
7. Do not run git commands. Do not touch anything outside this repository except /tmp.
8. When finished, reply with a short summary: files written, validator result, any spec problems, anything you would improve with more time.

## Palette use guide (ramps are dark->light)
neutrals ink,g1..g4,white | red r1..r4 | fire o1..o3,y1,y2 | green n1..n4 | cyan c1..c4 | blue b1..b4 | purple p1..p4 | sand/brown s1..s5 | magenta m1,m2.
Player ship family: blues/cyan/white with orange engine flame. Components: each a distinct accent (C1 red, C2 yellow, C3 cyan/green, C4 magenta/purple) on the same grey-blue hull.
Enemies: reds, oranges, purples, magentas, greens — never the player's blue/cyan hull so friend/foe is instantly readable.
Enemy bullets: hot pink/white/orange to contrast on all terrain. Terrain stays lower contrast (desaturated) so sprites pop.
