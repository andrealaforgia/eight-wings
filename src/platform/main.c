/* Eight Wings — platform entry point: window, GL, input, audio, fixed 60 Hz loop. */
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "core/stepper.h"
#include "game/game.h"
#include "game/sim.h"
#include "platform/audio.h"
#include "platform/renderer.h"
#include "platform/scene.h"
#include "platform/autoshots.h"

typedef struct {
  uint32_t seed;
  int start_stage;
  int scale;
  bool god, debug, no_audio, fullscreen, demo_first, help;
  const char* shots_dir;
  const char* asset_dir;
} Options;

static void usage(void) {
  printf("Eight Wings\n"
         "  --seed N        deterministic seed (default 1985)\n"
         "  --stage N       start at stage 1..3 (debug)\n"
         "  --scale N       initial window scale; default: the largest 256:224 window that fits the desktop\n"
         "  --fullscreen    start fullscreen\n"
         "  --god           invulnerable player (debug)\n"
         "  --debug         start with the debug overlay (F3 toggles)\n"
         "  --no-audio      silent\n"
         "  --assets DIR    asset directory (default ./assets)\n"
         "  --autoshots DIR run an automated scripted session and save real screenshots to DIR\n"
         "Keys: arrows/WASD move, Space/Z fire, X/LShift formation, Esc pause, Enter start, M mute, F fullscreen, F3 debug\n");
}

static Options parse(int argc, char** argv) {
  Options o = {1985, 0, 0, false, false, false, false, false, false, NULL, "assets"};
  for (int i = 1; i < argc; i++) {
    const char* a = argv[i];
    if (!strcmp(a, "--seed") && i + 1 < argc) o.seed = (uint32_t)strtoul(argv[++i], NULL, 10);
    else if (!strcmp(a, "--stage") && i + 1 < argc) o.start_stage = atoi(argv[++i]) - 1;
    else if (!strcmp(a, "--scale") && i + 1 < argc) o.scale = atoi(argv[++i]);
    else if (!strcmp(a, "--god")) o.god = true;
    else if (!strcmp(a, "--debug")) o.debug = true;
    else if (!strcmp(a, "--no-audio")) o.no_audio = true;
    else if (!strcmp(a, "--fullscreen")) o.fullscreen = true;
    else if (!strcmp(a, "--assets") && i + 1 < argc) o.asset_dir = argv[++i];
    else if (!strcmp(a, "--autoshots") && i + 1 < argc) o.shots_dir = argv[++i];
    else if (!strcmp(a, "--help") || !strcmp(a, "-h")) o.help = true;
    else { fprintf(stderr, "unknown option %s\n", a); o.help = true; }
  }
  if (o.scale < 0) o.scale = 0;
  return o;
}

static long load_hiscore(void) {
  char* dir = SDL_GetPrefPath("eight-wings", "eight-wings");
  if (!dir) return 0;
  char path[1100];
  snprintf(path, sizeof path, "%shiscore.txt", dir);
  SDL_free(dir);
  FILE* f = fopen(path, "r");
  long v = 0;
  if (f) { if (fscanf(f, "%ld", &v) != 1) v = 0; fclose(f); }
  return v < 0 ? 0 : v;
}

static void save_hiscore(long v) {
  char* dir = SDL_GetPrefPath("eight-wings", "eight-wings");
  if (!dir) return;
  char path[1100];
  snprintf(path, sizeof path, "%shiscore.txt", dir);
  SDL_free(dir);
  FILE* f = fopen(path, "w");
  if (f) { fprintf(f, "%ld\n", v); fclose(f); }
}

static Input read_input(void) {
  const Uint8* k = SDL_GetKeyboardState(NULL);
  Input in = {0};
  in.up = k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W];
  in.down = k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S];
  in.left = k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A];
  in.right = k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D];
  in.fire = k[SDL_SCANCODE_SPACE] || k[SDL_SCANCODE_Z];
  in.formation = k[SDL_SCANCODE_X] || k[SDL_SCANCODE_LSHIFT] || k[SDL_SCANCODE_RSHIFT];
  in.start = k[SDL_SCANCODE_RETURN] || k[SDL_SCANCODE_KP_ENTER];
  in.pause = k[SDL_SCANCODE_ESCAPE];
  return in;
}

static void drain_audio(Audio* audio, Game* g, Music* current) {
  for (int i = 0; i < g->sfx_count; i++) audio_play(audio, g->sfx_queue[i]);
  g->sfx_count = 0;
  if (g->music_request != *current) {
    *current = g->music_request;
    audio_music(audio, *current);
  }
  audio_set_volume(audio, g->settings.music_vol, g->settings.sfx_vol, g->settings.muted);
}

int main(int argc, char** argv) {
  Options opt = parse(argc, argv);
  if (opt.help) { usage(); return 0; }
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
  IMG_Init(IMG_INIT_PNG);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
  if (opt.fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  /* Big by default: the largest window with the game's aspect ratio that fits ~92% of the usable desktop. */
  int win_w, win_h;
  SDL_Rect usable;
  if (opt.scale == 0 && SDL_GetDisplayUsableBounds(0, &usable) == 0 && usable.w > 0) {
    float f = fminf((float)usable.w * 0.94f / SCREEN_W, (float)usable.h * 0.90f / SCREEN_H);
    if (f < 3.0f) f = 3.0f;
    win_w = (int)(SCREEN_W * f);
    win_h = (int)(SCREEN_H * f);
  } else {
    int sc = opt.scale ? opt.scale : 4;
    win_w = SCREEN_W * sc;
    win_h = SCREEN_H * sc;
  }
  SDL_Window* win = SDL_CreateWindow("Eight Wings", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_w, win_h, flags);
  if (!win) { fprintf(stderr, "window: %s\n", SDL_GetError()); return 1; }
  Renderer* r = renderer_create(win, opt.asset_dir);
  if (!r) return 1;
  Audio* audio = opt.no_audio || opt.shots_dir ? NULL : audio_create(opt.asset_dir);

  Game* g = (Game*)calloc(1, sizeof(Game));
  g->god_mode = opt.god;
  g->debug_overlay = opt.debug;
  g->hiscore = load_hiscore();
  game_init(g, opt.seed);

  int rc = 0;
  if (opt.shots_dir) {
    rc = autoshots_run(r, win, g, opt.shots_dir, opt.seed);
  } else {
    if (opt.start_stage > 0) game_new_run(g, opt.start_stage);
    bool running = true;
    double prev = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
    double acc = 0;
    long frame = 0;
    Music cur_music = MUS_NONE;
    long saved_hi = g->hiscore;
    while (running) {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
        if (e.type == SDL_KEYDOWN && !e.key.repeat) {
          if (e.key.keysym.scancode == SDL_SCANCODE_F3) g->debug_overlay = !g->debug_overlay;
          if (e.key.keysym.scancode == SDL_SCANCODE_M) g->settings.muted = !g->settings.muted;
          if (e.key.keysym.scancode == SDL_SCANCODE_F) {
            bool fs = SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN_DESKTOP;
            SDL_SetWindowFullscreen(win, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
          }
        }
      }
      double now = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
      /* the engine's stepper clamps a stalled frame (max 0.25 s) and splits it into steps <= 1/60 s;
         their sum is the time we feed a fixed-tick accumulator, so the simulation always advances in exact 1/60 s ticks */
      double step;
      int n = stepper_plan(now - prev, 1.0 / TICKS_PER_SEC, 0.25, &step);
      acc += step * n;
      prev = now;
      int ticks = (int)(acc * TICKS_PER_SEC);
      if (ticks > 8) { ticks = 8; acc = 0; } else acc -= (double)ticks / TICKS_PER_SEC;
      for (int t = 0; t < ticks; t++) {
        Input in = read_input();
        game_tick(g, &in);
        drain_audio(audio, g, &cur_music);
      }
      int w, h;
      SDL_GetWindowSize(win, &w, &h);
      renderer_begin(r, w, h);
      scene_draw(r, g, frame++);
      renderer_end(r);
      SDL_GL_SwapWindow(win);
      if (g->hiscore != saved_hi && (g->mode == MODE_GAMEOVER || g->mode == MODE_TITLE)) { save_hiscore(g->hiscore); saved_hi = g->hiscore; }
    }
    save_hiscore(g->hiscore);
  }
  free(g);
  audio_destroy(audio);
  renderer_destroy(r);
  SDL_DestroyWindow(win);
  IMG_Quit();
  SDL_Quit();
  return rc;
}
