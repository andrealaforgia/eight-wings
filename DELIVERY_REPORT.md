# Delivery report

## What was implemented (all of it exercised by the checks below)
A complete 3-stage arcade shooter with title screen, controls page, attract-mode demo, pause menu (resume, music/SFX volume, mute, quit), game over and restart,
high-score persistence, 8-way movement, shooting, lives and invulnerability, 8 dockable ship modules (colour-coded, each with its own weapon) that visibly reshape the ship, walking dinosaurs (raptor packs, rex) that leave skeletons behind, a formation attack
(plus a phoenix form at full assembly), 7 air-enemy types, 6 ground-target types, 3 multi-phase bosses with telegraphed beams, scrolling procedurally generated terrain in
3 themes, scoring/extra lives, escalating difficulty with looping, debug overlay, deterministic seeds, 23 synthesised sound effects and 7 generated music loops.
Documentation: README, docs/GAMEPLAY.md, docs/ARCHITECTURE.md, docs/ASSET_GENERATION.md, AUTONOMOUS_PROGRESS.md, THIRD_PARTY.md.

## Checks executed (macOS arm64) and results
| Check | Command | Result |
|---|---|---|
| Build from a fresh `git clone` | `make` | success, no warnings (`-Wall -Wextra -Wshadow -Wconversion -pedantic`) |
| Unit + scenario tests | `make test` | `ALL TESTS PASSED`: player/docking (56 checks), formation (231), enemies (24), ground/boss (51), level/game flow (9305), scenarios (13) |
| Sanitizers | `make sanitize` (ASan + UBSan) | passed, no reports |
| Asset contract | `python3 tools/validate_assets.py` | 352 sprite files, 0 problems; atlas bounds/overlap/pixel-equality verified |
| Reproducibility | rerun `gen_assets.py`, `pack_atlas.py`, `gen_audio.py` | `git status` clean: byte-identical output |
| Scenario: pilot (god mode) plays 6 minutes | `test_scenario` | 669 kills, 8/8 modules docked, 24 formation attacks, 3 bosses beaten, one full loop completed (finished on stage 1 of loop 2) |
| Scenario: each stage's boss beatable | `test_scenario` | all 3 bosses defeated from a stage-specific start |
| Scenario: unassisted pilot loses → game over | `test_scenario` | reaches `MODE_GAMEOVER` |
| Determinism | `test_scenario`, `test_ground_boss`, `test_level_game` | same seed ⇒ identical hash/level/boss bullets; different seed differs |
| Real-game smoke test | `./eight-wings` ran 5 s with audio, no errors | OK |
| Real screenshots | `make screenshots` | 17 PNGs in `screenshots/` captured from the running OpenGL game; no capture failed |

Screenshot inspection: I personally viewed title, stage-1 action, phoenix formation, boss 1 core phase, stage-2 archipelago, boss 3, and the pause menu. I found and fixed clipped text
(title tagline, pause help, controls line) and a wrong boss sprite index; the pause/controls/title strings were re-captured after the fix but not all 17 images were re-viewed individually.

## Round 2 changes (verified the same way)
Window auto-sizes to the desktop (fractional scaling when integer scaling would waste >10%); 8 modules with 2 counter-rotating formation rings and phoenix at full assembly; dinosaurs that hunt/amble,
avoid water, spit fire (rex) and leave bones (tested: bones stop moving, scroll with terrain, are not collidable); Codex graphics pass. `make test` passes (382 sprite files valid), 19 screenshots re-captured and the dinosaur, bones and 8-module screenshots viewed.
Boss HP was raised because the 8-module volley (10 damage) is far stronger. Not re-tuned by a human.

## Automated validation vs. human playtesting
Everything above is **automated**. **No human has played this build.** Difficulty tuning, game feel, audio quality (I cannot listen) and visual polish in motion are unverified.
The built-in pilot dies early on some seeds (0.4–2.4 min unassisted), which indicates stage 1 may be harder than intended for a weak player, or just that the pilot is poor.

## Incomplete / deviations
- Codex was available and used (`codex-cli 0.164.0`); all graphics came from its generators. Audio is Claude-authored, not Codex.
- Not studied from reference material: arcade-exact rules (see AUTONOMOUS_PROGRESS.md). Linux and gamepad untested/absent. Build uses Make, not CMake.
- Bosses 2 and 3 share boss 1's socket offsets.

## Commands
```sh
make                # build
make run            # play  (or ./eight-wings --seed 42 --stage 2 --debug)
make test           # all tests + asset validation
make sanitize       # tests under ASan/UBSan
make assets         # regenerate art + atlas
make audio          # regenerate audio
make screenshots    # capture real screenshots
```
