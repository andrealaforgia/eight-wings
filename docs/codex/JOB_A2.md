# JOB A2 — ship (8 modules!), fx, pickups, ui
Files you own: `tools/assetgen/gen_ship.py`, `gen_fx.py`, `gen_ui.py` (+ private helpers `_joba_*.py`). Groups: ship, fx, pickups, ui.
* The player ship must become a showpiece: refine core + banking frames. Design EIGHT add-on modules (C1..C8, layout in the spec header) so that when ALL are docked the
  composite looks like one coherent, much larger warship: wide swept wings (C1+C5, C2+C6), nose cannon C3 flanked by canards C7/C8, rear crown C4.
  Each module needs its own accent colour so a player can tell them apart and see which are attached: C1 red, C2 yellow, C3 green, C4 purple, C5 orange, C6 cyan, C7 pink/magenta, C8 white/silver-blue.
  Write a private script compositing core + all 8 at the spec offsets (48x64 canvas, enlarge 8x) and make sure joins are seamless; view it.
* pod_1..pod_8: capsules with big readable numerals 1..8, colour-matched to their module's accent. ui_comp 8 icons likewise.
* Improve phoenix, shots, all FX (explosions with real fireball volume and debris pixels, ship_death, dock_flash), clouds, logo, font (keep legibility).
