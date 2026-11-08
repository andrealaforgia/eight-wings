# Architecture

```
src/core/      rng, collision (AABB), stepper          shared, dependency-free (adapted/copied from super-fabio-bro)
src/game/      the deterministic simulation            NO SDL, NO OpenGL, no allocation in the loop
src/platform/  SDL2 window/input/audio, OpenGL renderer, scene drawing, autoshots harness
tests/         headless C tests linking only core+game
tools/         Python: asset generators (Codex), atlas packer, validator, audio synthesiser
assets/        spec/ (the contract), generated/ (per-sprite PNGs), atlas.png/.txt, audio/*.wav
```

## Simulation (`src/game`)
One `Game` struct holds all state in fixed-size arrays (bullets, enemies, ground, pickups, fx, spawn script, terrain map). `game_tick(g, input)` advances
exactly 1/60 s. Rendering and audio only *read* it; sound is requested through `sfx_queue`/`music_request`. Modules, each small and cohesive:
`game.c` (modes, stage flow, collisions, scoring), `player.c` (movement, weapons, docking, formation), `enemy.c`, `ground.c`, `boss.c`, `level.c`
(spawn script + procedural terrain with liquid autotiling), `demo_ai.c` (attract/test pilot). `sim.h` is the internal API between them.
Determinism: one xorshift RNG per run, seeded from `--seed`; level generation uses its own seeded streams. Tests assert identical results for identical seeds.
Ground objects live in **world** coordinates (`screen_y = 224 - wy + scroll`) so they never drift from the terrain.

## Platform (`src/platform`)
* `main.c`: window, GL 2.1 legacy context (fixed-function; runs on macOS and Linux), input from the keyboard state (movement + fire never conflict),
  fixed-timestep accumulator fed through the engine `stepper_plan`.
* `renderer.c`: 256×224 logical canvas drawn in an integer-scaled, letterboxed viewport; one texture atlas with NEAREST filtering; a CPU vertex batch
  (≈ one draw call per blend mode) with normal and additive blending, rotation, tint and per-sprite alpha. Positions snap to the pixel grid.
* `scene.c`: draws terrain tiles, ground, pickups, parallax clouds, shadows, enemies, bosses, the composite ship, bullets, FX, HUD, menus, debug overlay.
* `audio.c`: SDL audio callback mixer: 16 SFX voices (max 3 of the same sound) + looping music, volume/mute from the game's `Settings`.
* `autoshots.c`: scripted session driven by the pilot that saves real framebuffer captures at semantic moments.
* `sprite_table.h` is generated from `assets/spec/sprites.spec`, so code and art share one contract.

## Reuse from `super-fabio-bro` (read-only reference; nothing there was modified)
| Reused (copied, attributed in THIRD_PARTY.md) | Adapted | Deliberately different |
|---|---|---|
| `collision` (AABB) used for every hit test; `stepper` used by the loop | `rng` (renamed, same xorshift32); Makefile layout, `sdl2-config`, test layout (C tests, Python runner/validators), sanitizer target, MIT licence conventions | direct OpenGL renderer (the reference uses `SDL_Renderer` and has no GL); own mixer; fixed arrays instead of `object_pool` (no heap in the loop); `animate`/`physics` not used because they depend on the engine's graphics-context types |

## Key decisions
* Make, not CMake: matches the reference project; CMake was not installed on the build machine.
* Legacy fixed-function GL: simplest portable path for a textured-quad renderer. (Deprecated on macOS but functional; it would be the first thing to port to a core profile.)
* A hit on a docked module loses that module only; a core hit loses everything (more forgiving than the arcade, keeps docking meaningful).
* Formation attack is gauge-driven (kills charge it) so it is an earned, explicit resource with testable cooldowns.
* No render interpolation: the sim and display both run at 60 Hz; on higher refresh rates frames repeat.
