#ifndef TEST_HELPERS_H_
#define TEST_HELPERS_H_
#include <stdlib.h>
#include <string.h>

#include "game/game.h"
#include "game/sim.h"
#include "test.h"

/* A fresh game in the middle of stage 0 with an EMPTY level script, so tests control everything. */
static inline Game* new_playing_game(uint32_t seed) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, seed);
  game_new_run(g, 0);
  g->spawn_count = 0;
  g->spawn_next = 0;
  g->phase = PH_PLAY;
  g->player.invuln = 0;
  return g;
}

static inline void tick_n(Game* g, Input in, int n) {
  for (int i = 0; i < n; i++) game_tick(g, &in);
}

static inline void clear_hostiles(Game* g) {
  memset(g->enemies, 0, sizeof g->enemies);
  memset(g->ebullets, 0, sizeof g->ebullets);
  memset(g->ground, 0, sizeof g->ground);
}

static inline int count_active_pbullets(const Game* g) {
  int n = 0;
  for (int i = 0; i < MAX_PBULLETS; i++) n += g->pbullets[i].active ? 1 : 0;
  return n;
}

static inline void give_components(Game* g, int n) {
  for (int i = 0; i < n; i++) {
    g->player.comp[i] = true;
    g->player.dock_t[i] = DOCK_TICKS;
  }
}
#endif
