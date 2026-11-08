Created `gen_ship.py`, `gen_fx.py`, `gen_ui.py` and `_joba_preview.py`, generating all 175 PNGs.

Ship, FX and pickups pass validation. UI has one failure: the validator rejects the correctly transparent ASCII space (`font.0.png`). It needs an empty-space exception; protected files remain untouched.

Verified byte-identical regeneration, distinct FX frames and connected ship assembly at the specified offsets. Inspected 4× contact sheets in `/tmp/terra-joba-{ship,fx,ui}.png`.

With more time, I’d check readability against moving terrain in-game.