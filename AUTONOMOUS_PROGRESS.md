# Autonomous progress log

## State: complete and playable (see DELIVERY_REPORT.md for the verified claims)

## Done
- [x] Inspected `super-fabio-bro` (Makefile + `sdl2-config`, C tests + Python validators, engine `rng`/`collision`/`stepper`/`object_pool`, SDL_Renderer graphics). Reused/adapted what fits; documented in docs/ARCHITECTURE.md and THIRD_PARTY.md.
- [x] Asset contract (spec, palette, pixlib, validator) → three parallel Codex jobs → 352 sprites, validated, packed into an atlas.
- [x] Deterministic simulation: player (8-way, bounds, weapons, lives, invulnerability, death), components 1–4 with docking animation and loss, formation attack
      (scatter/hold/reunite/cooldown, phoenix at full assembly), 7 air enemies, 6 ground targets, 3 bosses (guns → core → enrage, beams), spawn script, procedural terrain
      with 3 themes and liquid autotiling, scoring/extra lives/hiscore, title/controls/attract demo/pause menu/game over/restart, difficulty loop.
- [x] Engine-based renderer (256×224 playfield in the engine view, atlas, additive blending, rotation), scene, HUD, debug overlay, SDL mixer, procedural SFX + 7 music loops.
- [x] Tests: 5 headless test binaries + asset validation; ASan/UBSan target; automated pilot scenarios; automated real screenshots (17).
- [x] Docs: README, GAMEPLAY, ARCHITECTURE, ASSET_GENERATION, this file, DELIVERY_REPORT.

## Decisions taken (no questions asked)
- Make instead of CMake (matches reference project; CMake not installed). Built on the sdl-2d-game-engine submodule, as super-fabio-bro is. Fixed arrays, no heap in the loop.
- Module hit loses that module (core hit loses everything). Formation gated by a kill-charged gauge, ≥2 docked modules; phoenix at 4.
- Bosses share mount offsets (guns ±28,+8; core 0,+14) because Codex was only told boss 1's.
- Opening difficulty was softened after the unassisted pilot died within a minute (bullet speed ×0.82, fire intervals ×1.33 at difficulty 0).
- Renamed to "EIGHT WINGS" in round 4; all art/audio/code are original.

## Known limitations / remaining
- No human playtesting happened. Feel, difficulty curve and audio quality are unverified by a person (I cannot hear audio; the pilot is a poor dodger so it is no proxy).
- Fidelity to the 1985 arcade is by design interpretation from general knowledge, not from studied reference material; rules differ (module loss, gauge-driven formation).
- Tested only on macOS (arm64, Apple clang). Linux build paths exist in the Makefile but were not run. macOS marks OpenGL deprecated (still works).
- Keyboard only (no gamepad), no key rebinding, no render interpolation above 60 Hz.
- Boss 2/3 gun/core sockets reuse boss 1's offsets; the art was verified to fit only by eye in screenshots.

## Round 4 correction: build on the owner's engine
The first rounds only borrowed two engine files and drew with raw SDL/OpenGL. Now `engine/` is the real sdl-2d-game-engine submodule (unmodified) and the
whole platform layer (window, drawing, input, audio, timing, command line) uses it; no source file includes SDL. Side effect: the engine's loader exposed that
`sfx_dino_die.wav` and `sfx_dino_spit.wav` had never been generated; they are now. Not verified by a human: sound and feel on a real display.
