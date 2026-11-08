# JOB B — enemies, enemy projectiles, ground targets, bosses
Sprite groups from the spec: `enemies`, `ground`, `bosses`.
Files to write: `tools/assetgen/gen_enemies.py`, `tools/assetgen/gen_ground.py`, `tools/assetgen/gen_bosses.py`.
Extra design notes:
- Air enemies face DOWN (toward the player). Each class has its own silhouette and palette: drone (round, green/teal pods), swoop (swept-wing red),
  diver (orange arrowhead), spinner (purple disc), wing (magenta sleek), bomber (heavy grey-red, gun pods), saucer (silver dome with chasing lights).
- Ground targets are viewed from straight above with a short cast shadow offset toward bottom-right (use 'g1' or 'ink' dithered, opaque only). Turret frame i aims at i*45 deg clockwise from up:
  draw the barrel by rotating a barrel shape (compute by line/polygon at that angle), keep the base identical.
- The bunker is THE key target: it hides the numbered docking modules. Make it distinctive (armoured hex hatch, hazard stripes, red beacon in frame 1, glowing yellow-white open hatch in frame 2).
- Bosses: huge, memorable and each unlike the others. They are drawn as body + 2 gun parts + 1 core placed by code at the offsets named in the spec.
  Paint the body so the mounts for the guns and the core look like recesses/sockets so assembly looks seamless. Give each boss a strong theme: boss1 walker fortress (grey/red/orange),
  boss2 segmented sea-serpent battleship (blue/cyan/white, armoured segments, fin spikes), boss3 mechanical queen mothership (purple/magenta/red, insectoid wings, crown).
  Symmetry (draw left half, mirror) is expected for bodies, but add asymmetrical damage/rivet detail sparingly.
- Enemy bullet orbs and missiles must stand out against sand/green/blue/dark-metal terrain: bright core, dark ink outline.
