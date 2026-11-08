/* Eight Wings: platform entry point. Window, drawing, input, audio, timing and the command line all come from
   the sdl-2d-game-engine (the engine/ submodule); this file only wires the deterministic game to them. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "command_line.h"
#include "controls.h"
#include "engine.h"
#include "event_system.h"
#include "events.h"
#include "fps_tracker.h"
#include "game/game.h"
#include "game/sim.h"
#include "graphics.h"
#include "keyboard.h"
#include "logger.h"
#include "platform/autoshots.h"
#include "platform/game_audio.h"
#include "platform/record.h"
#include "platform/renderer.h"
#include "platform/scene.h"
#include "stepper.h"

/* The buttons of this game: bits of the engine's controls mask. */
enum {
  BTN_UP = 1u << 0, BTN_DOWN = 1u << 1, BTN_LEFT = 1u << 2, BTN_RIGHT = 1u << 3,
  BTN_FIRE = 1u << 4, BTN_FORMATION = 1u << 5, BTN_START = 1u << 6, BTN_PAUSE = 1u << 7,
  BTN_DEBUG = 1u << 8, BTN_MUTE = 1u << 9, BTN_FULLSCREEN = 1u << 10
};

static const key_binding_t kBindings[] = {
    {BTN_UP, {KEY_UP, KEY_W, KEY_NONE, KEY_NONE}},
    {BTN_DOWN, {KEY_DOWN, KEY_S, KEY_NONE, KEY_NONE}},
    {BTN_LEFT, {KEY_LEFT, KEY_A, KEY_NONE, KEY_NONE}},
    {BTN_RIGHT, {KEY_RIGHT, KEY_D, KEY_NONE, KEY_NONE}},
    {BTN_FIRE, {KEY_SPACE, KEY_Z, KEY_NONE, KEY_NONE}},
    {BTN_FORMATION, {KEY_X, KEY_LSHIFT, KEY_RSHIFT, KEY_NONE}},
    {BTN_START, {KEY_RETURN, KEY_KP_ENTER, KEY_NONE, KEY_NONE}},
    {BTN_PAUSE, {KEY_ESCAPE, KEY_NONE, KEY_NONE, KEY_NONE}},
    {BTN_DEBUG, {KEY_F3, KEY_NONE, KEY_NONE, KEY_NONE}},
    {BTN_MUTE, {KEY_M, KEY_NONE, KEY_NONE, KEY_NONE}},
    {BTN_FULLSCREEN, {KEY_F11, KEY_F, KEY_NONE, KEY_NONE}},
};

typedef struct {
  int seed;
  int stage;
  int width, height;
  const char* record_dir;
  bool god, debug, no_audio;
  const char* shots_dir;
  const char* asset_dir;
} Options;

static Input to_input(uint32_t held) {
  Input in = {0};
  in.up = held & BTN_UP;
  in.down = held & BTN_DOWN;
  in.left = held & BTN_LEFT;
  in.right = held & BTN_RIGHT;
  in.fire = held & BTN_FIRE;
  in.formation = held & BTN_FORMATION;
  in.start = held & BTN_START;
  in.pause = held & BTN_PAUSE;
  return in;
}

static bool hiscore_path(char* out, size_t n) {
  const char* home = getenv("HOME");
  if (!home) return false;
  snprintf(out, n, "%s/.eight-wings-hiscore", home);
  return true;
}

static long load_hiscore(void) {
  char path[1100];
  long v = 0;
  FILE* f = hiscore_path(path, sizeof path) ? fopen(path, "r") : NULL;
  if (f) { if (fscanf(f, "%ld", &v) != 1) v = 0; fclose(f); }
  return v < 0 ? 0 : v;
}

static void save_hiscore(long v) {
  char path[1100];
  FILE* f = hiscore_path(path, sizeof path) ? fopen(path, "w") : NULL;
  if (f) { fprintf(f, "%ld\n", v); fclose(f); }
}

/* The audio listens to the game: each sound or music request is an engine event. */
static void on_sfx(const game_event_t* event, void* audio) { audio_play(audio, (Sfx)(event->type - GAME_EVENT_SFX)); }
static void on_music(const game_event_t* event, void* audio) { audio_music(audio, (Music)(event->type - GAME_EVENT_MUSIC)); }

int main(int argc, char** argv) {
  Options opt = {.seed = 1985, .asset_dir = "assets"};
  const custom_option_t custom[] = {
      {"seed", OPTION_INT, &opt.seed, "deterministic seed (default 1985)"},
      {"stage", OPTION_INT, &opt.stage, "start at stage 1..3 (debug)"},
      {"width", OPTION_INT, &opt.width, "window width (default: as tall as the screen)"},
      {"height", OPTION_INT, &opt.height, "window height (default: as tall as the screen)"},
      {"god", OPTION_FLAG, &opt.god, "invulnerable player (debug)"},
      {"debug", OPTION_FLAG, &opt.debug, "start with the debug overlay (F3 toggles)"},
      {"no-audio", OPTION_FLAG, &opt.no_audio, "silent"},
      {"assets", OPTION_STRING, &opt.asset_dir, "asset directory (default ./assets)"},
      {"record", OPTION_STRING, &opt.record_dir, "play a full run with the demo pilot and dump frames + a sound-event log to this directory (see tools/record_video.py)"},
      {"autoshots", OPTION_STRING, &opt.shots_dir, "run a scripted session and save real screenshots (BMP) to this directory"},
  };
  const size_t custom_count = sizeof(custom) / sizeof(custom[0]);
  command_line_options_t engine = default_command_line_options();
  engine.window_mode = WINDOWED;
  engine.vsync = true;
  engine.volume = 96;
  const unsigned offered = CLI_WINDOW | CLI_VOLUME | CLI_GRAPHICS_INFO | CLI_SHOW_FPS;
  command_line_result_t parsed = parse_command_line(argc, argv, offered, &engine, custom, custom_count);
  if (parsed.status == COMMAND_LINE_ERROR) {
    fprintf(stderr, "%s\n", parsed.message);
    return 2;
  }
  if (parsed.status == COMMAND_LINE_HELP) {
    printf("usage: eight-wings [options]\n");
    print_help(offered, &engine, custom, custom_count);
    printf("keys: arrows/WASD move, Space/Z fire, X/Shift formation, Enter start, Esc pause, M mute, F11 or F full screen, F3 debug\n");
    return 0;
  }
  if (opt.stage > 0) opt.stage -= 1;
  if (!engine_start()) {
    fprintf(stderr, "cannot start the engine: %s\n", engine_last_error());
    return 1;
  }
  if (engine.graphics_info) {
    print_graphics_info();
    engine_stop();
    return 0;
  }
  engine_set_log_level(LOG_LEVEL_WARN);
  graphics_config_t config = default_graphics_config("Eight Wings");
  config.display = engine.display;
  config.display_mode = engine.display_mode;
  config.window_mode = engine.window_mode;
  config.vsync = engine.vsync;
  config.view_height = SCREEN_H;
  if (opt.width <= 0 && opt.height <= 0 && config.window_mode == WINDOWED) {
    /* Fill the screen vertically and size the width to the playfield: open a window that fills the usable screen,
       read its height, close it, and open the real one at that height with the playfield's aspect ratio. */
    graphics_context_ptr probe = create_graphics_context(&config);
    if (probe) {
      begin_frame(probe, 0, 0, 0);
      float pixels = graphics_view_scale(probe) * (float)SCREEN_H; /* output pixels */
      terminate_graphics_context(probe);
      display_mode_t mode;
      float dpi = 1.0f;
      if (get_display_mode(config.display, 0, &mode) && mode.height > 0) dpi = floorf(pixels / (float)mode.height + 0.5f);
      if (dpi < 1.0f) dpi = 1.0f;
      opt.height = (int)(pixels / dpi);
      opt.width = opt.height * SCREEN_W / SCREEN_H;
    }
  }
  config.width = opt.width;
  config.height = opt.height;
  graphics_context_ptr gfx = create_graphics_context(&config);
  if (!gfx) {
    fprintf(stderr, "cannot open a window: %s\n", engine_last_error());
    engine_stop();
    return 1;
  }
  Renderer* r = renderer_create(gfx, opt.asset_dir, "assets_cooked", "engine/core/graphics/fonts/arcade-font.png");
  if (!r) {
    terminate_graphics_context(gfx);
    engine_stop();
    return 1;
  }
  Audio* audio = opt.no_audio || opt.shots_dir || opt.record_dir ? NULL : audio_create(opt.asset_dir, engine.volume);

  static event_system_t events;
  events = create_event_system();
  if (audio) {
    for (int i = 1; i < SFX_COUNT; i++) subscribe(&events, GAME_EVENT_SFX + i, on_sfx, audio);
    for (int i = 1; i < MUS_COUNT; i++) subscribe(&events, GAME_EVENT_MUSIC + i, on_music, audio);
  }
  if (opt.record_dir && !record_attach(&events, opt.record_dir)) {
    fprintf(stderr, "cannot write the event log in %s\n", opt.record_dir);
    return 1;
  }
  Game* g = (Game*)calloc(1, sizeof(Game));
  g->events = &events;
  g->god_mode = opt.god;
  g->debug_overlay = opt.debug;
  g->hiscore = load_hiscore();
  game_init(g, (uint32_t)opt.seed);

  int rc = 0;
  if (opt.record_dir) {
    rc = record_run(r, g, opt.record_dir);
  } else if (opt.shots_dir) {
    rc = autoshots_run(r, g, opt.shots_dir, (uint32_t)opt.seed);
  } else {
    if (opt.stage > 0) game_new_run(g, opt.stage);
    bool running = true;
    double prev = get_clock_seconds();
    double acc = 0;
    long frame = 0;
    long saved_hi = g->hiscore;
    controls_t controls;
    memset(&controls, 0, sizeof controls);
    fps_tracker_t fps = create_fps_tracker();
    while (running) {
      if (drain_events() == QUIT_EVENT) running = false;
      double now = get_clock_seconds();
      fps_tracker_frame(&fps, now);
      controls_update(&controls, keyboard_read(kBindings, sizeof kBindings / sizeof kBindings[0]));
      if (controls_pressed(&controls, BTN_DEBUG)) g->debug_overlay = !g->debug_overlay;
      if (controls_pressed(&controls, BTN_MUTE)) g->settings.muted = !g->settings.muted;
      if (controls_pressed(&controls, BTN_FULLSCREEN)) toggle_fullscreen(gfx);
      /* the engine's stepper clamps a stalled frame (max 0.25 s) and splits it into steps <= 1/60 s;
         their sum feeds a fixed-tick accumulator, so the simulation always advances in exact 1/60 s ticks */
      double step;
      int n = stepper_plan(now - prev, 1.0 / TICKS_PER_SEC, 0.25, &step);
      acc += step * n;
      prev = now;
      int ticks = (int)(acc * TICKS_PER_SEC);
      if (ticks > 8) { ticks = 8; acc = 0; } else acc -= (double)ticks / TICKS_PER_SEC;
      for (int t = 0; t < ticks; t++) {
        Input in = to_input(controls.held);
        game_tick(g, &in);
      }
      audio_set_volume(audio, g->settings.music_vol, g->settings.sfx_vol, g->settings.muted);
      renderer_begin(r);
      scene_draw(r, g, frame++);
      if (engine.show_fps) {
        char buf[32];
        snprintf(buf, sizeof buf, "%d FPS", (int)(fps_tracker_current(&fps) + 0.5));
        renderer_text(r, buf, 2, SCREEN_H - 10, 0.7f, 1.0f, 0.7f, 1.0f);
      }
      renderer_end(r);
      renderer_present(r);
      if (g->hiscore != saved_hi && (g->mode == MODE_GAMEOVER || g->mode == MODE_TITLE)) { save_hiscore(g->hiscore); saved_hi = g->hiscore; }
    }
    save_hiscore(g->hiscore);
  }
  game_free(g);
  free(g);
  audio_destroy(audio);
  renderer_destroy(r);
  terminate_graphics_context(gfx);
  engine_stop();
  return rc;
}
