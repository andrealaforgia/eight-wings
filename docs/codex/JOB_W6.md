# JOB W6 — weapon projectiles and weapon capsules
Files you own: ONLY new code in `tools/assetgen/gen_ship.py` plus private helpers `tools/assetgen/_w6_*.py`. Groups: ship (only the 4 new shots) and pickups (only the 5 new capsules).
`gen_ship.py` was heavily reworked last round (ship, modules): ADD your generators and wire them into `generate()`, but do not change any existing sprite: every existing
file under assets/generated must remain byte-identical. Do NOT edit palette.py (use existing colours; if you truly need more, say so in your reply).
Sprites: shot_pellet, shot_laser, shot_missile, shot_wave, wp_vulcan, wp_spread, wp_laser, wp_missile, wp_wave per the spec lines.
Shots must be instantly distinguishable from each other and from the existing shot_p/shot_w/shot_f/shot_ph at 1x, bright and readable on desert/jungle/metal terrain.
The wp_* capsules are hexagonal with a big readable letter and a colour family matching the weapon, clearly different in silhouette from the round numbered pod_N capsules.
