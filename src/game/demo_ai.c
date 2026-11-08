/* Attract-mode / automated-test pilot: dodges bullets, chases pods and bunkers, fires, and triggers formations. */
#include "game/sim.h"

void demo_ai(const Game* g, Input* out) {
  *out = (Input){0};
  const Player* p = &g->player;
  out->fire = true;
  if (p->state != PS_ALIVE) return;
  float tx = p->pos.x, ty = PLAYER_SPAWN_Y;

  /* target: nearest pickup, else the nearest ground bunker / enemy above us */
  float best = 1e9f;
  POOL_EACH(const Pickup, k, g->pickups) {
    float d = fabsf(k->pos.x - p->pos.x) + fabsf(k->pos.y - p->pos.y) * 0.3f;
    if (d < best) { best = d; tx = k->pos.x; ty = k->pos.y; }
  }
  if (best > 1e8f) {
    POOL_EACH(const Ground, e, g->ground) {
      if (e->hp <= 0) continue;
      float sy = SCREEN_H - e->pos.y + g->scroll;
      if (sy < 10 || sy > p->pos.y) continue;
      float d = fabsf(e->pos.x - p->pos.x) + (p->pos.y - sy) * 0.2f - (e->type == GT_BUNKER ? 120.0f : 0);
      if (d < best) { best = d; tx = e->pos.x; ty = PLAYER_SPAWN_Y; }
    }
    POOL_EACH(const Enemy, e, g->enemies) {
      if (e->pos.y > p->pos.y) continue;
      float d = fabsf(e->pos.x - p->pos.x) * 0.8f + (p->pos.y - e->pos.y) * 0.1f;
      if (d < best) { best = d; tx = e->pos.x; ty = PLAYER_SPAWN_Y; }
    }
    if (g->boss.active) {
      const Boss* b = &g->boss;
      tx = b->pos.x;
      if (b->phase == 0) {
        for (int i = 0; i < 2; i++)
          if (b->gun_hp[i] > 0) { float gx, gy; boss_gun_pos(b, i, &gx, &gy); if (fabsf(gx - p->pos.x) < fabsf(tx - p->pos.x) || tx == b->pos.x) tx = gx; }
      }
      ty = 175;
    }
  }
  /* dodge: sum of repulsion from nearby threats */
  float rx = 0, ry = 0;
  POOL_EACH(const EBullet, b, g->ebullets) {
    float dx = p->pos.x - b->pos.x, dy = p->pos.y - b->pos.y;
    if (dy < -8 || dy > 56 || fabsf(dx) > 22) continue;
    float w = 1.0f / (0.6f + dx * dx * 0.05f + fabsf(dy) * 0.08f);
    rx += (dx >= 0 ? 1.0f : -1.0f) * w;
    ry += (dy >= 0 ? 0.3f : -0.3f) * w;
  }
  POOL_EACH(const Enemy, e, g->enemies) {
    float dx = p->pos.x - e->pos.x, dy = p->pos.y - e->pos.y;
    if (fabsf(dx) > 34 || dy < -10 || dy > 60) continue;
    rx += (dx >= 0 ? 1.0f : -1.0f) * 1.2f;
  }
  float mx = (tx - p->pos.x) * 0.08f + rx * 1.6f;
  float my = (ty - p->pos.y) * 0.05f + ry * 1.0f;
  if (mx > 0.25f) out->right = true;
  if (mx < -0.25f) out->left = true;
  if (my > 0.35f) out->down = true;
  if (my < -0.35f) out->up = true;
  if (player_can_start_formation(g)) out->formation = (g->tick % 2) == 0;
}
