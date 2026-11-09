# ROUND 6 — MORE EXCITEMENT (Claude Code -> Codex)

Owner feedback: "make the game a bit more exciting: diversify the enemies, add a few more bigger dinos, make the weapon on the starship of various types."
`assets/spec/sprites.spec` already lists the NEW sprites at the bottom (section "ROUND 6"); sizes, frame counts and groups are BINDING (ART_SCALE 4: 4 texels
per logical px). All rules of BRIEF_COMMON.md, BRIEF_ROUND3.md, BRIEF_ROUND4.md apply (read them): procedural stdlib-only Python, deterministic via
`pixlib.rng_for`, binary alpha, palette names only, 1-2 texel outlines, 6-8 tonal steps per material, strong silhouettes, top-left light, no
anti-aliasing. Match the existing visual language of the sprites already in the repo (look at `assets/generated/enemies`, `ground` and `ship` and at the
generator code first: same outline weight, same shading depth, same detail level), so the new art looks like it belongs in the same game.
Do NOT edit the spec, pixlib.py, validator, or files owned by the other job. Do not run git. Run `python3 tools/gen_assets.py` (or your module's generate())
and `python3 tools/validate_assets.py --group <g>` for your groups (the validator may complain about the other job's sprites until it finishes: ignore those).
Make a private contact sheet (`tools/contact_sheet.py` exists) enlarged AND at game size (nearest downscale by 4) and look at it; iterate on quality.
Friend/foe readability: enemies use reds/oranges/purples/magentas/greens/teals, never the player's blue/cyan hull. Enemy art faces DOWN, dinosaurs face DOWN.
When finished reply with a short summary (files, validator result, what you would improve).
