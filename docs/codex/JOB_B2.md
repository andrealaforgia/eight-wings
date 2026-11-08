# JOB B2 — enemies, ground (incl. NEW DINOSAURS + BONES), bosses
Files you own: `tools/assetgen/gen_enemies.py`, `gen_ground.py`, `gen_bosses.py` (+ private helpers `_jobb_*.py`). Groups: enemies, ground, bosses.
* NEW and important: **walking dinosaurs** (as in classic arcade shooters, where ground creatures stomp around and leave their skeletons behind when shot).
  `g_raptor` (20x20, fast, 4-frame run cycle) and `g_rex` (32x32, big, 4-frame stomp cycle): viewed from above, facing DOWN, believable anatomy (head, jaws, spine ridge, tail,
  clawed legs alternating per frame), colour so they stand out from desert/jungle/metal terrain. `g_bones_s` / `g_bones_l`: matching skeleton lying on the ground (skull, spine, ribs, leg bones, tail), bone-white with dark gaps,
  same pose as the live creature so it reads as "that thing's remains".
* Redo every enemy, ground target and boss for greater depth and character per the Round-2 brief. Bosses: still 96x56 bodies with the socket recesses described in the spec; add far more
  mechanical detail, silhouette spikes/fins, cockpit glow, damage-free but richly detailed hulls.
