# ROUND 3 — HI-RES REDRAW (Claude Code -> Codex)

Owner feedback: "The sprites are too coarse-grained. Refine them to look more crisp."
Cause: the art was authored at 1 texel = 1 logical pixel on a 256x224 playfield. Fix: **every sprite is now authored at 2x density**.
`assets/spec/sprites.spec` has ALREADY been rewritten: every width/height is doubled (ship_core 32x48, tiles 32x32, font 16x16 ...). The game draws each
sprite at (w/2 x h/2) logical px, so on screen nothing changes size, but there are 4x as many texels to spend. Sizes in the spec are binding.

Rules from BRIEF_COMMON.md and BRIEF_ROUND2.md still apply (stdlib only, deterministic, binary alpha, `python3 tools/validate_assets.py --group <g>`), EXCEPT:
* **Palette may grow.** You may append new named colours to `tools/assetgen/palette.py` (keep every existing name/value; target <= 96 total). Add more
  tonal steps per material (5-6 for hull metal, 4-5 for terrain, 5+ for fire) so shading is smooth and crisp. Coordinate: Job A owns palette.py edits;
  Jobs B and C may only use colours already there plus those Job A adds, so if you need a colour not present, derive it as an RGBA tuple in your own
  helper module and tell the validator via a tiny `EXTRA` dict in your helper (validator accepts only palette.py colours - so if blocked, ask Job A's
  colours: Job A, please add generously and early: extra ramps for steel, red, orange, yellow, green, cyan, purple, sand, grass, water, stone, bone, fire).
* **Do not just upscale the old art.** Re-draw it: crisp 1-texel outlines (= half a logical pixel), clean anti-alias-free curved edges, real curves
  (use circles/ellipses/polygon fills/Bezier-like arcs, not stair-steps), thin panel lines, rivets, cockpit glass with specular dots, fine hatching,
  engine glow gradients made of 4-6 discrete steps. Think "16-bit console sprite sheet at 2x", not "chunky 8-bit".
* Write drawing code **in terms of a SCALE constant / float coordinates** where convenient, so shapes are defined once and rasterised crisply.
* Frames of an animation must stay the same size and keep the same silhouette centre.
* Tiles must still tile seamlessly (edges wrap) and liquid autotile masks must still match edge-to-edge at the new size.
* Keep readability at the real on-screen size: after you finish, downscale a contact sheet 2x with nearest and check the sprite still reads.
Verify with `python3 tools/validate_assets.py --group <g>` and `python3 tools/contact_sheet.py <g> 4`; iterate until it clearly looks sharper and richer than before.
Do NOT edit files outside your job's list. Reply with a short summary.
