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
  S_STAGE2, S_BOSS2, S_STAGE3, S_BOSS3, S_DINO, S_BONES, S_NEWFOES, S_BIGDINO, S_WSPREAD, S_WLASER, S_WMISSILE, S_WWAVE, S_GAMEOVER, S_COUNT
};

static Shot shots[S_COUNT] = {{"01_title", false}, {"02_controls", false}, {"03_stage1_intro", false}, {"04_stage1_action", false}, {"05_pod_released", false}, {"06_docking", false},
                              {"07_assembled_ship", false}, {"08_formation_attack", false}, {"09_phoenix_formation", false}, {"10_boss1_guns", false}, {"11_boss1_core", false},
                              {"12_pause_menu", false}, {"13_stage2_archipelago", false}, {"14_boss2_hydra", false}, {"15_stage3_fortress", false}, {"16_boss3_mother_core", false}, {"17_dinosaurs_walking", false}, {"18_dinosaur_bones", false}, {"20_new_enemies", false}, {"21_big_dinosaur", false},
                              {"22_weapon_spread", false}, {"23_weapon_laser", false}, {"24_weapon_missile", false}, {"25_weapon_wave", false},
                              {"19_game_over", false}};

static long frame_no;

static void capture(Renderer* r, const Game* g, const char* dir, int idx) {
  renderer_begin(r);
  scene_draw(r, g, frame_no);
  renderer_end(r);
  char path[1024];
  snprintf(path, sizeof path, "%s/%s.bmp", dir, shots[idx].name);
  bool ok = renderer_save_bmp(r, path);
  renderer_present(r);
  shots[idx].done = ok;
  printf("%s %s\n", ok ? "captured" : "FAILED  ", path);
}

static int count_enemies(const Game* g) { return (int)pool_get_active_count(&g->enemies); }
static int count_ebullets(const Game* g) { return (int)pool_get_active_count(&g->ebullets); }
static int docked(const Game* g) { int n = 0; for (int i = 0; i < NUM_COMPONENTS; i++) n += player_comp_docked(g, i); return n; }

int autoshots_run(Renderer* r, Game* g, const char* dir, uint32_t seed) {
  (void)seed;
  Input none = {0};
  for (int i = 0; i < 30; i++) { game_tick(g, &none); frame_no++; }
  capture(r, g, dir, S_TITLE);
  Input pause = {.pause = true};
  game_tick(g, &pause); game_tick(g, &none);
  for (int i = 0; i < 20; i++) game_tick(g, &none);
  capture(r, g, dir, S_CONTROLS);
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
    if (!shots[S_INTRO].done && g->phase == PH_INTRO && g->phase_tick == 70) capture(r, g, dir, S_INTRO);
    if (!shots[S_ACTION].done && g->stage == 0 && g->scroll > 700 && count_enemies(g) >= 3 && count_ebullets(g) >= 2) capture(r, g, dir, S_ACTION);
    if (!shots[S_POD].done) POOL_EACH(const Pickup, k, g->pickups) if (k->number > 0 && k->pos.y > 50 && k->pos.y < 140) { capture(r, g, dir, S_POD); break; }
    if (!shots[S_DOCK2].done && player_component_count(g) >= 3 && !player_comp_docked(g, player_component_count(g) - 1)) capture(r, g, dir, S_DOCK2);
    if (!shots[S_ASSEMBLED].done && docked(g) == NUM_COMPONENTS && g->phase != PH_BOSS && g->player.form_state == FORM_NONE) capture(r, g, dir, S_ASSEMBLED);
    if (!shots[S_FORMATION].done && p->form_state == FORM_HOLD && p->form_t == FORM_SCATTER_TICKS + 40 && !player_is_phoenix(g)) capture(r, g, dir, S_FORMATION);
    if (!shots[S_PHOENIX].done && player_is_phoenix(g) && p->form_state == FORM_HOLD && p->form_t == FORM_SCATTER_TICKS + 70) capture(r, g, dir, S_PHOENIX);
    if (!shots[S_BOSS1_GUNS].done && g->stage == 0 && g->boss.active && g->boss.phase == 0 && g->boss.entering == 0 && g->boss.t == 150) capture(r, g, dir, S_BOSS1_GUNS);
    if (!shots[S_BOSS1_CORE].done && g->stage == 0 && g->boss.active && g->boss.phase == 1 && !g->boss.dying && g->boss.t % 7 == 0 && count_ebullets(g) > 5) capture(r, g, dir, S_BOSS1_CORE);
    if (!pause_done && g->stage == 0 && g->scroll > 1500 && g->phase == PH_PLAY) {
      game_tick(g, &pause); game_tick(g, &none);
      Input down = {.down = true};
      game_tick(g, &down); game_tick(g, &none);
      capture(r, g, dir, S_PAUSE);
      game_tick(g, &pause); game_tick(g, &none);
      pause_done = true;
    }
    if (!shots[S_STAGE2].done && g->stage == 1 && g->scroll > 600 && count_enemies(g) >= 3) capture(r, g, dir, S_STAGE2);
    if (!shots[S_BOSS2].done && g->stage == 1 && g->boss.active && g->boss.phase >= 1 && !g->boss.dying && count_ebullets(g) > 6) capture(r, g, dir, S_BOSS2);
    if (!shots[S_STAGE3].done && g->stage == 2 && g->scroll > 600 && count_enemies(g) >= 3) capture(r, g, dir, S_STAGE3);
    if (!shots[S_BOSS3].done && g->stage == 2 && g->boss.active && g->boss.phase >= 1 && !g->boss.dying && count_ebullets(g) > 8) capture(r, g, dir, S_BOSS3);
    POOL_EACH(const Ground, d, g->ground) {
      if ((d->type != GT_RAPTOR && d->type != GT_REX)) continue;
      float sy = sim_ground_sy(g, d->pos.y);
      if (sy < 50 || sy > 150) continue;
      if (d->hp > 0 && !shots[S_DINO].done) capture(r, g, dir, S_DINO);
      if (d->hp <= 0 && !shots[S_BONES].done) capture(r, g, dir, S_BONES);
    }
    if (!shots[S_NEWFOES].done) {
      int n = 0;
      POOL_EACH(const Enemy, e, g->enemies) n += (e->type >= ET_WEAVER && e->pos.y > 20 && e->pos.y < 150);
      if (n >= 3 && g->phase != PH_BOSS) capture(r, g, dir, S_NEWFOES);
    }
    if (!shots[S_BIGDINO].done)
      POOL_EACH(const Ground, d, g->ground) {
        float sy = sim_ground_sy(g, d->pos.y);
        if ((d->type == GT_TRIKE || d->type == GT_SPINO || d->type == GT_BRONTO) && d->hp > 0 && sy > 60 && sy < 130 && p->pos.y > sy + 30) { capture(r, g, dir, S_BIGDINO); break; }
      }
    bool all = true;
    for (int i = 0; i < S_WSPREAD; i++) all = all && shots[i].done;
    if (all) break;
  }
  /* weapon showcase: an empty sky, the fully assembled ship, fire held down, two gunships to shoot at */
  static const int wshot[4] = {S_WSPREAD, S_WLASER, S_WMISSILE, S_WWAVE};
  for (int w = WT_SPREAD; w <= WT_WAVE; w++) {
    game_new_run(g, 0);
    g->god_mode = true;
    g->spawn_count = 0;
    g->spawn_next = 0;
    g->phase = PH_PLAY;
    g->player.weapon = w;
    for (int i = 0; i < NUM_COMPONENTS; i++) { g->player.comp[i] = true; g->player.dock_t[i] = DOCK_TICKS; }
    enemy_spawn(g, ET_GUNSHIP, 70, 20, 0, 1);
    enemy_spawn(g, ET_GUNSHIP, 190, 20, 0, 1);
    Input hold = {.fire = true};
    for (int t = 0; t < 70; t++) { game_tick(g, &hold); frame_no++; }
    capture(r, g, dir, wshot[w - WT_SPREAD]);
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
  if (g->mode == MODE_GAMEOVER) capture(r, g, dir, S_GAMEOVER);
  int missing = 0;
  for (int i = 0; i < S_COUNT; i++) if (!shots[i].done) { printf("MISSING  %s\n", shots[i].name); missing++; }
  return missing ? 1 : 0;
}
