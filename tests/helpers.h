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
  pool_reset(&g->enemies);
  pool_reset(&g->ebullets);
  pool_reset(&g->ground);
}

static inline int count_active_pbullets(const Game* g) {
  return (int)pool_get_active_count(&g->pbullets);
}

/** Adds a player bullet at (x, y) moving (vx, vy) pixels per tick. */
static inline PBullet* add_pbullet(Game* g, PBulletKind kind, float x, float y, float vx, float vy, int dmg) {
  PBullet* b = POOL_NEW(PBullet, g->pbullets);
  b->kind = kind;
  b->pos = point(x, y);
  sim_set_velocity(&b->vel, vx, vy);
  b->dmg = dmg;
  b->pierce = false;
  b->age = b->life = 0;
  b->ox = x;
  b->phase = b->amp = 0;
  return b;
}

/** Replaces all player bullets with one resting at (x, y) that deals `dmg`. */
static inline void shoot_at(Game* g, float x, float y, int dmg) {
  pool_reset(&g->pbullets);
  add_pbullet(g, PB_NORMAL, x, y, 0, 0, dmg);
}

/** The first (lowest-slot) active object of a pool, or NULL. */
static inline void* first_active(const object_pool_t* pool) {
  size_t cursor = 0;
  return pool_next_active(pool, &cursor);
}

static inline void give_components(Game* g, int n) {
  for (int i = 0; i < n; i++) {
    g->player.comp[i] = true;
    g->player.dock_t[i] = DOCK_TICKS;
  }
}
#endif
