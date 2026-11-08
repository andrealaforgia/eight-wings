# JOB A — player ship, components, projectiles, FX, pickups, UI, font, logo
Sprite groups from the spec: `ship`, `fx`, `pickups`, `ui`.
Files to write: `tools/assetgen/gen_ship.py` (group ship + pickups), `tools/assetgen/gen_fx.py` (group fx), `tools/assetgen/gen_ui.py` (group ui incl. font and logo).
Extra design notes:
- The ship must look iconic: a sleek blue/white delta fighter. When C1..C4 are fused around it (offsets listed in the spec) the combined silhouette
  must read as one bigger, meaner craft. Test this: write a private script that composites core + all four components at the given offsets onto a 48x64 canvas and check the join looks right.
- phoenix: 4-frame flaming golden/orange/red bird with spread wings, symmetrical, readable at 32x32, wings flap (up/mid/down/mid).
- explosions: classic arcade frame progressions (flash -> fireball -> embers/smoke), fully opaque pixels only, ragged round shapes.
- font: 95 glyphs ASCII 32..126, 8x8 cells, glyph pixels drawn in 'white', legible digits and capital letters (score displays), define bitmaps as data strings in the script.
- logo: 192x48 'EIGHT WINGS', chunky 3-D arcade lettering with bevel/outline, gradient-free banded shading, two rows or one row as fits.
