# ROUND 5 — ONE COHERENT SHIP (Claude Code -> Codex)

Owner feedback: "I don't like the way the ship composes. The parts are meh... I want a ship where each part contributes to make the overall ship
nice-looking and coherent." Today the core and the eight modules (C1..C8) were drawn as separate objects, so the assembled ship reads as a pile of
add-ons. Redesign them as ONE aircraft.

## What to build (files you own: `tools/assetgen/gen_ship.py` and private helpers `tools/assetgen/_ship5_*.py`; nothing else)
Sprite names, sizes, frame counts and the module layout are BINDING (see `assets/spec/sprites.spec`, ART_SCALE 4: 4 texels per logical px). Redraw only:
`ship_core` (2 frames), `ship_core_bank_l`, `ship_core_bank_r`, `ship_c1`..`ship_c8` (2 frames each). Keep palette names; you may append colours to
`palette.py` only if essential (append only). Do not touch phoenix, shots, pods, ui, fx.

Module centres relative to the core centre, in logical px (+y down), as the game places them:
C1 (-12,+2) left inner wing, C2 (+12,+2) right inner wing, C3 (0,-16) nose, C4 (0,+15) tail, C5 (-22,+6) left outer wing, C6 (+22,+6) right outer wing,
C7 (-10,-12) left canard, C8 (+10,-12) right canard. Sprite sizes (logical px = texels/4) as in the spec. Game code will not change these offsets,
so your art must fit them: neighbouring pieces overlap by ~1-2 logical px (4-8 texels) and must visually continue each other's lines.

## Recommended method (designs the whole, then slices it)
1. Compose the FULLY ASSEMBLED ship once, on a single large master canvas (e.g. 256x256 texels, centre = core centre), as one designed aircraft:
   a swept-wing heavy fighter with one fuselage spine, a continuous leading edge running from canard -> inner wing -> outer wing, panel lines that
   cross the module boundaries and re-align exactly, a consistent light direction (top-left), shared engine nozzles (a nozzle row on C4 plus
   small nozzles at the trailing edge of each wing), and a nose that grows out of the fuselage. Think of a single heroic silhouette (arrowhead /
   falcon / "eight wings" idea: the wings step outward in tiers, the canards and the nose are the front tier, the tail fins the back tier).
2. Give every module an ownership mask (a polygon region of the master) following a natural seam: a panel line, a hinge, a bolted joint. Seams
   should look like deliberate docking latches (small bright connector pins, a clamp), NOT arbitrary cuts.
3. Slice the master into the sprites by ownership. Draw each piece's 1-2 texel dark outline AFTER slicing, so that any subset still looks finished
   (core alone must read as a complete small fighter; core + C1 + C2 must read as a complete bigger fighter, etc.). The outline between two docked
   pieces must not show as a thick black gap: keep it as a thin dark seam only, or let the neighbour overlap hide it (the game draws modules
   after the core, in index order).
4. One material language for all pieces: same hull blue-grey ramp, same white trim, same cockpit-glass cyan, same engine glow orange. Per-module accent
   colour (C1 red, C2 yellow, C3 green, C4 purple, C5 orange, C6 cyan, C7 pink, C8 silver-blue) stays, but ONLY as a stripe / intake / light on a
   shared hull, at most ~15-20% of the piece's area, so players can still tell modules apart and the ship stays harmonious.
5. Symmetry: left/right pairs mirror each other in shape (accent colours differ). Core: draw half and mirror.
6. Each wing/nose/tail module must also read as a purposeful part when scattered in formation attack (they orbit alone): give each a clear
   gun/muzzle or engine so it looks like a little craft of its own.
7. Banking frames: `ship_core_bank_l/r` narrow the fuselage and wings like a banked fighter (the modules are not banked by the game, so keep the
   core's width within its footprint).
8. Engine flames (frame 0/1) flicker; module frames flicker only at their nozzles; stable silhouette.

## Verify
`python3 tools/validate_assets.py --group ship` (exit 0), then write a private script that composites core + all 8 modules at the offsets above, enlarged,
plus partial assemblies (core only; core+C1+C2; core+C1..C4; all eight), and view them at 1x/2x logical size (nearest downscale by 4) to confirm they
read well at game size. Iterate until every partial assembly looks like one coherent ship. Binary alpha, deterministic output (`pixlib.rng_for`),
1-2 texel outlines. Do not run git. Reply with a short summary and what you'd improve with more time.
