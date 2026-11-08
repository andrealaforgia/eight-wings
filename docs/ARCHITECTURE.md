# Architecture

```
engine/        git submodule: andrealaforgia/sdl-2d-game-engine (the same engine super-fabio-bro uses). Never modified here.
src/game/      the deterministic simulation            NO platform library, no allocation in the loop
src/platform/  wiring onto the engine: renderer, audio, main loop, scene drawing, autoshots harness
tests/         headless C tests linking src/game + the engine's pure parts (rng, collision, stepper, controls, mixer)
tools/         Python: asset generators (Codex), atlas packer, validator, audio synthesiser
assets/        spec/ (the contract), generated/ (per-sprite PNGs), atlas.png/.txt, audio/*.wav
assets_cooked/ atlas.rgba, built by `make` from the generated art (git-ignored)
```

## Simulation (`src/game`)
One `Game` struct holds all state in fixed-size arrays (bullets, enemies, ground, pickups, fx, spawn script, terrain map). `game_tick(g, input)` advances
exactly 1/60 s. Rendering and audio only *read* it; sound is requested through `sfx_queue`/`music_request`. Modules, each small and cohesive:
`game.c` (modes, stage flow, collisions, scoring), `player.c` (movement, weapons, docking, formation), `enemy.c`, `ground.c`, `boss.c`, `level.c`
(spawn script + procedural terrain with liquid autotiling), `demo_ai.c` (attract/test pilot). `sim.h` is the internal API between them.
Determinism: one xorshift RNG per run, seeded from `--seed`; level generation uses its own seeded streams. Tests assert identical results for identical seeds.
Ground objects live in **world** coordinates (`screen_y = 224 - wy + scroll`) so they never drift from the terrain.

## Platform (`src/platform`): everything runs on the engine
The game is built on the shared sdl-2d-game-engine, consumed exactly as `super-fabio-bro` does: the engine's own makefile builds `libsdl2d.a`, and the
game compiles against its public headers only. No file in this repo includes SDL (`scripts/check_boundary.py` and the stub headers in
`scripts/boundary_stubs/` enforce it, as in the reference).
* `main.c`: engine start/stop, command line (`parse_command_line` with the game's own options), `create_graphics_context` (a view 224 units tall whose
  width follows the window), key bindings (`key_binding_t` + `keyboard_read`) feeding `controls_t`, events (`drain_events`), `get_clock_seconds` and
  `stepper_plan` driving a fixed 1/60 s tick accumulator, `toggle_fullscreen`, `fps_tracker`.
* `renderer.c`: sprites through `render_sprite` from the atlas texture (`texture_from_pixels`, crisp scaling), plus a second additive copy of the atlas for
  glows and hit flashes; rotation, tint, alpha and flips map onto `draw_options_t`; rectangles use `draw_filled_rect_alpha`. The 256-wide playfield is centred
  in the engine's view and the extra width becomes dark side bars. Art is authored at 4 texels per logical pixel (`ART_SCALE`).
* `scene.c`: draws terrain tiles, ground, pickups, parallax clouds, shadows, enemies, bosses, the composite ship, bullets, FX, HUD, menus, debug overlay.
* `game_audio.c`: engine audio context: SFX in engine sound slots (`load_sound`, `play_sound_at_volume`); music loops through the engine's music-source callback.
* `autoshots.c`: scripted session driven by the pilot; frames are saved with the engine's `save_frame_bmp` and converted to PNG by `tools/bmp2png.py`.
* `sprite_table.h` is generated from `assets/spec/sprites.spec`, so code and art share one contract.

## What comes from the engine
Window and rendering (`graphics_context`, `texture`, `drawing_primitives`), input (`keyboard`, `controls`, `events`), audio (`audio`, `mixer`), timing (`clock`,
`stepper`), the command-line parser, logging, and in the simulation `rng_t` and `aabb_collision` (every hit test). Not used: `object_pool` (the simulation keeps
fixed arrays so nothing allocates in the loop), `physics`/`animate` (movement here is scripted per entity), the event system (sound is requested through
`sfx_queue`).

## Key decisions
* Make, not CMake: matches the reference project and the engine.
* Engine, not raw SDL/GL: the game uses the owner's shared 2D engine as a dependency (submodule), like super-fabio-bro. The engine is never edited from this repo.
* A hit on a docked module loses that module only; a core hit loses everything (more forgiving than the arcade, keeps docking meaningful).
* Formation attack is gauge-driven (kills charge it) so it is an earned, explicit resource with testable cooldowns.
* No render interpolation: the sim and display both run at 60 Hz; on higher refresh rates frames repeat.
