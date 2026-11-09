 assert not changed,(label,changed)
 print(f'"'{len(expected)} {label} PNGs byte-identical after regeneration')
PY" in /Users/andrealaforgia/dev/games/terra-cresta
 succeeded in 1381ms:
generating gen_ship
checked 37 sprite files, 0 problems
checked 28 sprite files, 0 problems
/tmp/sheet_ship.png
/tmp/sheet_pickups.png
382 existing PNGs byte-identical after regeneration
17 new PNGs byte-identical after regeneration

codex
Added nine weapon sprites across 17 PNGs, using [gen_ship.py](/Users/andrealaforgia/dev/games/terra-cresta/tools/assetgen/gen_ship.py) and [_w6_weapons.py](/Users/andrealaforgia/dev/games/terra-cresta/tools/assetgen/_w6_weapons.py).

Ship and pickups validators pass with zero problems. Regeneration is byte-identical; all 382 existing PNGs remain unchanged.

Reviewed enlarged and game-size previews. No spec problems or new colours needed. Further improvement would focus on checking visibility during busy in-game combat.
tokens used
70,943
Added nine weapon sprites across 17 PNGs, using [gen_ship.py](/Users/andrealaforgia/dev/games/terra-cresta/tools/assetgen/gen_ship.py) and [_w6_weapons.py](/Users/andrealaforgia/dev/games/terra-cresta/tools/assetgen/_w6_weapons.py).

Ship and pickups validators pass with zero problems. Regeneration is byte-identical; all 382 existing PNGs remain unchanged.

Reviewed enlarged and game-size previews. No spec problems or new colours needed. Further improvement would focus on checking visibility during busy in-game combat.
