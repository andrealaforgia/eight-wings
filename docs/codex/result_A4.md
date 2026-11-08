With more time, I’d refine fireball turbulence and review animation timing in-game.
tokens used
113,794
Redrew all assigned assets at native 4× density: eight ship modules, phoenix, shots, pickups, effects, logo and font.

Updated `gen_ship.py`, `gen_fx.py`, `gen_ui.py` and `_joba_preview.py`; added `_joba_ultra.py`, `_joba_type.py` and `_joba_check.py`.

All four validators pass: 195 PNGs, zero problems. Regeneration is byte-identical. Docking connectivity, stable hull silhouettes and distinct animation frames pass. Enlarged and logical-size previews inspected.

One brief inconsistency: the ship spans 56 logical pixels, exceeding the requested 48-pixel preview. Both cropped and uncropped previews are in `/tmp`.

With more time, I’d refine fireball turbulence and review animation timing in-game.
