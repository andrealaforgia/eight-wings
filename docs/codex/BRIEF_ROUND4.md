# ROUND 4 — ULTRA HI-RES REDRAW (Claude Code -> Codex)

Owner feedback: "Improve the resolution even more." Round 3 doubled texel density (ART_SCALE 2). Round 4 doubles it AGAIN: **ART_SCALE 4**, i.e. 4 texels per
logical pixel per axis (16x the original texel count). `assets/spec/sprites.spec` is ALREADY updated: every size is 2x the Round 3 size (ship_core 64x96,
tiles 64x64, font 32x32). Sizes are binding. The game draws each sprite at (w/4 x h/4) logical px, so on-screen size is unchanged.

All rules of BRIEF_ROUND3.md apply (read it). Round 3 generators exist in your files: **do not upscale their output**; redefine geometry at the new
resolution (SCALE constants/float coordinates, real curves, rasterised polygons/ellipses/bezier-like arcs), and spend the extra texels on genuine detail:
micro panel lines, rivets, bevelled edges, layered cockpit glass, specular highlights, 6-8 tonal steps per material, fine feather/scale/bone/brick/leaf
texture, smooth glow ramps, crisp 1-2 texel outlines (outline thickness should stay visually ~0.25-0.5 logical px, i.e. 1-2 texels, NOT 4).
Palette may keep growing (append only, keep existing names; <= 160 colours total; Job A owns palette.py edits and should add ramps early).
Binary alpha, determinism, seamless tiles, matching liquid autotile edges, distinct animation frames, validator passing: all still required.
Verify: `python3 tools/validate_assets.py --group <g>`, `python3 tools/contact_sheet.py <g> 2`; also view a nearest-downscaled (by 4) version to confirm
it still reads at game size. Do NOT edit files outside your job's list. Reply with a short summary.
