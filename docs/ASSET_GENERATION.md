# Asset generation: Claude Code × Codex

## Workflow actually used
1. **Claude Code wrote the contract**: `assets/spec/sprites.spec` (every sprite's name, exact size, frame count, group, art direction), the master palette
   `tools/assetgen/palette.py` (38 colours), the drawing toolkit `tools/assetgen/pixlib.py` (canvas, polygon/line/ellipse, outline, mirror, PNG writer, `emit()`),
   the validator `tools/validate_assets.py`, and the generator runner `tools/gen_assets.py`. Briefs: `docs/codex/BRIEF_COMMON.md`, `JOB_A/B/C.md`.
2. **Codex (`codex exec`, three parallel jobs, workspace-write sandbox)** wrote the generators and produced the pixels. Each Codex reply is saved in
   `docs/codex/result_*.md`.
   * Job A → `tools/assetgen/gen_ship.py`, `gen_fx.py`, `gen_ui.py`: player ship + 4 modules + phoenix + shots, explosions/sparks/docking flash/clouds, pods/medals, logo, 95-glyph font, HUD icons (175 PNGs).
   * Job B → `gen_enemies.py`, `gen_ground.py`, `gen_bosses.py`: 7 air enemies, enemy bullets/missile/beam, 6 ground targets + rubble, 3 bosses with cores and guns (78 PNGs).
   * Job C → `gen_tiles.py`: 99 terrain tiles for 3 themes, including 16-mask liquid autotiles with matching shorelines and connecting roads.
3. **Claude Code integrated**: `tools/pack_atlas.py` packs everything into `assets/atlas.png` + `atlas.txt` and generates `src/platform/sprite_table.h`; the renderer and scene
   draw from it. Claude inspected contact sheets (`tools/contact_sheet.py`) and real in-game screenshots.
4. **Automated checks** (`make test` → `tools/validate_assets.py`): every file exists, exact dimensions, RGBA PNG, alpha strictly 0/255, every opaque pixel is a palette colour,
   no empty sprites (except the space glyph), animation frames not all identical, tiles fully opaque, atlas rectangles in-bounds/non-overlapping and atlas pixels identical to the sources.
   Claude fixed one *validator* bug found by Codex's report (it rejected the blank space glyph).

Claude did not hand-edit any generated sprite. Spec limitation noted by Codex: bosses 2 and 3 share boss 1's gun/core mount offsets (the spec only named them for boss 1); the game uses the same offsets for all three.
Audio is not from Codex: `tools/audiogen/gen_audio.py` (Claude Code) synthesises SFX and algorithmically composes the music.

## Round 2 (graphics pass, 8 modules, dinosaurs)
After the first playable build the owner asked for better graphics, many more ship pieces and walking dinosaurs. Claude Code extended the contract (ship_c5..c8, pod_5..8, 8-frame ui_comp,
g_raptor, g_rex, g_bones_s/l), wrote `docs/codex/BRIEF_ROUND2.md` + `JOB_A2/B2/C2.md` (concrete art direction based on the real screenshots), and Codex rewrote the generators in three parallel jobs
(195 + 88 + 99 sprites; helper modules `_joba_*`, `_jobb_*`, `_c_checks.py`). Results: `docs/codex/result_*2.md`. The validator and atlas checks passed (382 sprite files) before integration.

## Reproduce
```sh
python3 tools/gen_assets.py        # all generators (deterministic; byte-identical on rerun)
python3 tools/pack_atlas.py        # atlas + metadata + sprite_table.h
python3 tools/validate_assets.py   # contract checks
python3 tools/contact_sheet.py ship 5   # magnified sheet in /tmp/sheet_ship.png
python3 tools/audiogen/gen_audio.py     # audio
```
Calling Codex again requires the Codex CLI: `codex exec -C . --sandbox workspace-write - < <(cat docs/codex/BRIEF_COMMON.md docs/codex/JOB_A.md)`.

## Round 3: hi-res redraw
Owner feedback: sprites too coarse. All art is now authored at 2x density (`ART_SCALE 2` in `renderer.h`; spec sizes doubled, e.g. `ship_core` 32x48).
The renderer draws each sprite at w/2 x h/2 logical px, so on-screen size and all gameplay are unchanged. Codex redrew every generator (briefs:
`docs/codex/BRIEF_ROUND3.md`, `run_round3.sh`; results `result_*3.md`). 382 files validate, regeneration is byte-identical. Not human-reviewed beyond screenshots.

## Round 5: one coherent ship
Owner feedback: the core and its eight modules looked like unrelated add-ons. Codex redesigned them (`docs/codex/BRIEF_ROUND5.md`, `run_round5.sh`)
by composing the fully assembled ship once as a single swept-wing aircraft and slicing it into the existing sprites by module, so panel lines,
leading edges and engines continue across the docking seams; each module keeps only an accent stripe on the shared hull. Sprite names, sizes
and module offsets are unchanged. Generator: `tools/assetgen/gen_ship.py` with `_ship5_aircraft.py`.

## Round 6: more excitement
Owner feedback: "make the game a bit more exciting: diversify the enemies, add a few more bigger dinos, make the weapon on the starship of various types."
Codex drew 23 new sprites in two parallel jobs (`docs/codex/BRIEF_ROUND6.md`, `JOB_E6.md`, `JOB_W6.md`, `run_round6.sh`): four flyers (`e_weaver`, `e_hunter`, `e_gunship`,
`e_splitter`), three big dinosaurs with skeletons (`g_trike`, `g_spino`, `g_bronto`, `g_bones_*`), four weapon shots (`shot_pellet`, `shot_laser`, `shot_missile`, `shot_wave`) and five
hexagonal weapon capsules (`wp_*`). Known weak spot: the dinosaur walk cycles move their legs only slightly between frames.
