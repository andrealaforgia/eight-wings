#include "platform/record.h"

#include <stdio.h>

#include "game/sim.h"
#include "platform/scene.h"

static FILE* g_log;
static long g_tick;

static void log_event(const game_event_t* event, void* user) {
  (void)user;
  if (!g_log) return;
  if (event->type >= GAME_EVENT_MUSIC) fprintf(g_log, "%ld M %d\n", g_tick, event->type - GAME_EVENT_MUSIC);
  else fprintf(g_log, "%ld S %d\n", g_tick, event->type - GAME_EVENT_SFX);
}

bool record_attach(event_system_t* events, const char* out_dir) {
  char path[1100];
  snprintf(path, sizeof path, "%s/events.txt", out_dir);
  g_log = fopen(path, "w");
  if (!g_log) return false;
  for (int i = 1; i < SFX_COUNT; i++) subscribe(events, GAME_EVENT_SFX + i, log_event, NULL);
  for (int i = 1; i < MUS_COUNT; i++) subscribe(events, GAME_EVENT_MUSIC + i, log_event, NULL);
  return true;
}

static long g_frames;

static bool step(Renderer* r, Game* g, const char* dir, const Input* in) {
  game_tick(g, in);
  g_tick++;
  if (g_tick % 2) return true;
  renderer_begin(r);
  scene_draw(r, g, g_tick);
  renderer_end(r);
  char path[1100];
  snprintf(path, sizeof path, "%s/f%06ld.bmp", dir, g_frames);
  /* the driver deletes frames as it encodes them; write to a temporary name first so it never reads a half-written file */
  char tmp[1110];
  snprintf(tmp, sizeof tmp, "%s.part", path);
  if (!renderer_save_bmp(r, tmp) || rename(tmp, path) != 0) return false;
  renderer_present(r);
  g_frames++;
  return true;
}

int record_run(Renderer* r, Game* g, const char* dir) {
  Input none = {0}, start = {.start = true};
  if (g_log) fprintf(g_log, "0 V %d %d %d\n", g->settings.music_vol, g->settings.sfx_vol, g->settings.muted ? 1 : 0);
  bool ok = true;
  for (int i = 0; i < 240 && ok; i++) ok = step(r, g, dir, &none); /* title screen */
  g->god_mode = true; /* the run ends when the game starts its second loop; the demo pilot is not a perfect player: god mode lets it show every stage and boss */
  ok = ok && step(r, g, dir, &start);
  for (long t = 0; t < 60L * 60 * 25 && ok && g->mode == MODE_PLAYING && !g->loop; t++) {
    Input in;
    demo_ai(g, &in);
    ok = step(r, g, dir, &in);
  }
  for (int i = 0; i < 300 && ok; i++) ok = step(r, g, dir, &none); /* the ending */
  if (g_log) fclose(g_log);
  g_log = NULL;
  char path[1100];
  snprintf(path, sizeof path, "%s/DONE", dir);
  FILE* f = fopen(path, "w");
  if (f) { fprintf(f, "%ld frames\n", g_frames); fclose(f); }
  printf("recorded %ld frames (%s)\n", g_frames, ok ? "ok" : "FAILED");
  return ok ? 0 : 1;
}
