#include "platform/autoshots.h"

#include <stdio.h>
#include <string.h>

#include "game/sim.h"
#include "platform/scene.h"

typedef struct {
  const char* name;
  bool done;
} Shot;

enum {
  S_TITLE, S_CONTROLS, S_INTRO, S_ACTION, S_POD, S_DOCK2, S_ASSEMBLED, S_FORMATION, S_PHOENIX, S_BOSS1_GUNS, S_BOSS1_CORE, S_PAUSE,
  S_STAGE2, S_BOSS2, S_STAGE3, S_BOSS3, S_DINO, S_BONES, S_GAMEOVER, S_COUNT
};

static Shot shots[S_COUNT] = {{"01_title", false}, {"02_controls", false}, {"03_stage1_intro", false}, {"04_stage1_action", false}, {"05_pod_released", false}, {"06_docking", false},
                              {"07_assembled_ship", false}, {"08_formation_attack", false}, {"09_phoenix_formation", false}, {"10_boss1_guns", false}, {"11_boss1_core", false},
                              {"12_pause_menu", false}, {"13_stage2_archipelago", false}, {"14_boss2_hydra", false}, {"15_stage3_fortress", false}, {"16_boss3_mother_core", false}, {"17_dinosaurs_walking", false}, {"18_dinosaur_bones", false},
                              {"19_game_over", false}};

static long frame_no;

static void capture(Renderer* r, SDL_Window* win, const Game* g, const char* dir, int idx) {
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  renderer_begin(r, w, h);
  scene_draw(r, g, frame_no);
  renderer_end(r);
  char path[1024];
  snprintf(path, sizeof path, "%s/%s.png", dir, shots[idx].name);
  bool ok = renderer_read_pixels(r, path);
  SDL_GL_SwapWindow(win);
  shots[idx].done = ok;
  printf("%s %s\n", ok ? "captured" : "FAILED  ", path);
  SDL_PumpEvents();
}

static int count_enemies(const Game* g) { int n = 0; for (int i = 0; i < MAX_ENEMIES; i++) n += g->enemies[i].active; return n; }
static int count_ebullets(const Game* g) { int n = 0; for (int i = 0; i < MAX_EBULLETS; i++) n += g->ebullets[i].active; return n; }
static int docked(const Game* g) { int n = 0; for (int i = 0; i < NUM_COMPONENTS; i++) n += player_comp_docked(g, i); return n; }

int autoshots_run(Renderer* r, SDL_Window* win, Game* g, const char* dir, uint32_t seed) {
  (void)seed;
  Input none = {0};
  for (int i = 0; i < 30; i++) { game_tick(g, &none); frame_no++; }
  capture(r, win, g, dir, S_TITLE);
  Input pause = {.pause = true};
  game_tick(g, &pause); game_tick(g, &none);
  for (int i = 0; i < 20; i++) game_tick(g, &none);
  capture(r, win, g, dir, S_CONTROLS);
  g->god_mode = true;
  Input start = {.start = true};
  game_tick(g, &start);
  game_tick(g, &none);

  long limit = 60L * 60 * 12;
  bool pause_done = false;
  for (long t = 0; t < limit && g->mode == MODE_PLAYING; t++) {
    Input in;
    demo_ai(g, &in);
    if (g->paused) { game_tick(g, &pause); game_tick(g, &none); }
    game_tick(g, &in);
    frame_no++;
    const Player* p = &g->player;
    if (!shots[S_INTRO].done && g->phase == PH_INTRO && g->phase_tick == 70) capture(r, win, g, dir, S_INTRO);
    if (!shots[S_ACTION].done && g->stage == 0 && g->scroll > 700 && count_enemies(g) >= 3 && count_ebullets(g) >= 2) capture(r, win, g, dir, S_ACTION);
    if (!shots[S_POD].done) for (int i = 0; i < MAX_PICKUPS; i++) if (g->pickups[i].active && g->pickups[i].number > 0 && g->pickups[i].pos.y > 50 && g->pickups[i].pos.y < 140) { capture(r, win, g, dir, S_POD); break; }
    if (!shots[S_DOCK2].done && player_component_count(g) >= 3 && !player_comp_docked(g, player_component_count(g) - 1)) capture(r, win, g, dir, S_DOCK2);
    if (!shots[S_ASSEMBLED].done && docked(g) == NUM_COMPONENTS && g->phase != PH_BOSS && g->player.form_state == FORM_NONE) capture(r, win, g, dir, S_ASSEMBLED);
    if (!shots[S_FORMATION].done && p->form_state == FORM_HOLD && p->form_t == FORM_SCATTER_TICKS + 40 && !player_is_phoenix(g)) capture(r, win, g, dir, S_FORMATION);
    if (!shots[S_PHOENIX].done && player_is_phoenix(g) && p->form_state == FORM_HOLD && p->form_t == FORM_SCATTER_TICKS + 70) capture(r, win, g, dir, S_PHOENIX);
    if (!shots[S_BOSS1_GUNS].done && g->stage == 0 && g->boss.active && g->boss.phase == 0 && g->boss.entering == 0 && g->boss.t == 150) capture(r, win, g, dir, S_BOSS1_GUNS);
    if (!shots[S_BOSS1_CORE].done && g->stage == 0 && g->boss.active && g->boss.phase == 1 && !g->boss.dying && g->boss.t % 7 == 0 && count_ebullets(g) > 5) capture(r, win, g, dir, S_BOSS1_CORE);
    if (!pause_done && g->stage == 0 && g->scroll > 1500 && g->phase == PH_PLAY) {
      game_tick(g, &pause); game_tick(g, &none);
      Input down = {.down = true};
      game_tick(g, &down); game_tick(g, &none);
      capture(r, win, g, dir, S_PAUSE);
      game_tick(g, &pause); game_tick(g, &none);
      pause_done = true;
    }
    if (!shots[S_STAGE2].done && g->stage == 1 && g->scroll > 600 && count_enemies(g) >= 3) capture(r, win, g, dir, S_STAGE2);
    if (!shots[S_BOSS2].done && g->stage == 1 && g->boss.active && g->boss.phase >= 1 && !g->boss.dying && count_ebullets(g) > 6) capture(r, win, g, dir, S_BOSS2);
    if (!shots[S_STAGE3].done && g->stage == 2 && g->scroll > 600 && count_enemies(g) >= 3) capture(r, win, g, dir, S_STAGE3);
    if (!shots[S_BOSS3].done && g->stage == 2 && g->boss.active && g->boss.phase >= 1 && !g->boss.dying && count_ebullets(g) > 8) capture(r, win, g, dir, S_BOSS3);
    for (int i = 0; i < MAX_GROUND; i++) {
      const Ground* d = &g->ground[i];
      if (!d->active || (d->type != GT_RAPTOR && d->type != GT_REX)) continue;
      float sy = sim_ground_sy(g, d->pos.y);
      if (sy < 50 || sy > 150) continue;
      if (d->hp > 0 && !shots[S_DINO].done) capture(r, win, g, dir, S_DINO);
      if (d->hp <= 0 && !shots[S_BONES].done) capture(r, win, g, dir, S_BONES);
    }
    bool all = true;
    for (int i = 0; i < S_GAMEOVER; i++) all = all && shots[i].done;
    if (all) break;
  }
  /* unassisted run on the hardest stage until the game is lost */
  g->god_mode = false;
  game_new_run(g, 2);
  for (long t = 0; t < 60L * 60 * 40 && g->mode == MODE_PLAYING; t++) {
    Input in;
    demo_ai(g, &in);
    game_tick(g, &in);
    frame_no++;
  }
  for (int i = 0; i < 100 && g->mode == MODE_GAMEOVER; i++) { game_tick(g, &none); frame_no++; }
  if (g->mode == MODE_GAMEOVER) capture(r, win, g, dir, S_GAMEOVER);
  int missing = 0;
  for (int i = 0; i < S_COUNT; i++) if (!shots[i].done) { printf("MISSING  %s\n", shots[i].name); missing++; }
  return missing ? 1 : 0;
}
