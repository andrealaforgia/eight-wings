#include "game/sim.h"

void enemy_half_size(EnemyType t, float* hw, float* hh) {
  switch (t) {
    case ET_BOMBER: *hw = 15; *hh = 11; break;
    case ET_SAUCER: *hw = 11; *hh = 7; break;
    default: *hw = 7; *hh = 7; break;
  }
}

int enemy_score(EnemyType t) {
  static const int s[ET_COUNT] = {100, 150, 200, 300, 120, 800, 400};
  return s[t];
}

int enemy_hp_for(const Game* g, EnemyType t) {
  static const int hp[ET_COUNT] = {1, 1, 1, 3, 1, 14, 5};
  int h = hp[t];
  if (h >= 3) h += g->difficulty / 2;
  return h;
}

Enemy* enemy_spawn(Game* g, EnemyType type, float x, float y, float p0, int dir) {
  Enemy* e = POOL_NEW(Enemy, g->enemies);
  if (!e) return NULL;
  e->type = type;
  e->pos = point(x, y);
  e->hp = enemy_hp_for(g, type);
  e->p0 = p0;
  e->p1 = x;
  e->dir = dir ? dir : 1;
  switch (type) {
    case ET_DRONE: sim_set_velocity(&e->vel, 0, 1.25f); break;
    case ET_SWOOP:
      e->p0 = (float)e->dir * 0.95f; /* heading, 0 = straight down */
      sim_set_velocity(&e->vel, sinf(e->p0) * 2.1f, cosf(e->p0) * 2.1f);
      break;
    case ET_DIVER: sim_set_velocity(&e->vel, 0, 1.7f); break;
    case ET_SPINNER: sim_set_velocity(&e->vel, 0, 0.7f); break;
    case ET_WING: sim_set_velocity(&e->vel, 0, 1.6f); break;
    case ET_BOMBER: sim_set_velocity(&e->vel, 0, 0.5f); break;
    case ET_SAUCER: sim_set_velocity(&e->vel, 0, 0.9f); break;
    default: break;
  }
  return e;
}

static void kill_enemy(Game* g, Enemy* e) {
  POOL_FREE(g->enemies, e);
  g->kills++;
  int pts = enemy_score(e->type);
  sim_add_score(g, pts);
  if (e->type == ET_BOMBER) {
    sim_fx(g, FX_EXPL_L, e->pos.x, e->pos.y, false);
    sim_fx_score(g, e->pos.x, e->pos.y - 10, pts);
    sim_pickup(g, 0, e->pos.x, e->pos.y);
    sim_sfx(g, SFX_EXPLODE_BIG);
    sim_charge_gauge(g, 10);
  } else {
    sim_fx(g, FX_EXPL_S, e->pos.x, e->pos.y, false);
    sim_sfx(g, SFX_ENEMY_DIE);
    sim_charge_gauge(g, 4);
    if (pts >= 300) sim_fx_score(g, e->pos.x, e->pos.y - 8, pts);
  }
}

void enemy_hit(Game* g, Enemy* e, int dmg) {
  e->hp -= dmg;
  e->flash = 3;
  if (e->hp <= 0) kill_enemy(g, e);
  else sim_sfx(g, SFX_HIT);
}

static void update_enemy(Game* g, Enemy* e) {
  e->age++;
  if (e->flash > 0) e->flash--;
  const Player* pl = &g->player;
  switch (e->type) {
    case ET_DRONE:
      sim_move(&e->pos, &e->vel);
      e->pos.x = e->p1 + 34.0f * sinf((float)e->age * 0.06f + e->p0);
      if (e->age == 55 + (int)(e->p0 * 7) % 20) sim_aimed_shot(g, e->pos.x, e->pos.y, 1.5f, 0);
      break;
    case ET_SWOOP:
      e->p0 -= (float)e->dir * 0.024f;
      sim_set_velocity(&e->vel, sinf(e->p0) * 2.1f, cosf(e->p0) * 2.1f);
      sim_move(&e->pos, &e->vel);
      if (e->age == 38) sim_aimed_shot(g, e->pos.x, e->pos.y, 1.6f, 0);
      break;
    case ET_DIVER:
      if (e->state == 0) {
        sim_move(&e->pos, &e->vel);
        if (e->pos.y >= 34.0f + e->p0) { e->state = 1; e->timer = 36; }
      } else if (e->state == 1) {
        if (--e->timer <= 0) {
          float dx = pl->pos.x - e->pos.x, dy = pl->pos.y - e->pos.y;
          if (pl->state != PS_ALIVE) { dx = 0; dy = 1; }
          float len = sqrtf(dx * dx + dy * dy);
          if (len < 1) len = 1;
          float s = 3.4f + 0.12f * (float)g->difficulty;
          sim_set_velocity(&e->vel, dx / len * s, dy / len * s);
          e->state = 2;
        }
      } else {
        sim_move(&e->pos, &e->vel);
      }
      break;
    case ET_SPINNER:
      sim_move(&e->pos, &e->vel);
      e->pos.x = e->p1 + 26.0f * cosf((float)e->age * 0.05f + e->p0);
      if (e->age > 40 && e->pos.y < 150 && (e->age % sim_fire_interval(g, 95)) == 0) {
        sim_aimed_shot(g, e->pos.x, e->pos.y, 1.7f, -0.35f);
        sim_aimed_shot(g, e->pos.x, e->pos.y, 1.7f, 0);
        sim_aimed_shot(g, e->pos.x, e->pos.y, 1.7f, 0.35f);
      }
      break;
    case ET_WING:
      if (e->state == 0) {
        sim_move(&e->pos, &e->vel);
        if (e->age >= 75) {
          e->state = 1;
          sim_set_velocity(&e->vel, (float)e->dir * 2.4f, 0.5f);
          sim_aimed_shot(g, e->pos.x, e->pos.y, 1.8f, 0);
        }
      } else {
        sim_set_velocity(&e->vel, sim_vx(&e->vel) * 1.012f, sim_vy(&e->vel));
        sim_move(&e->pos, &e->vel);
      }
      break;
    case ET_BOMBER:
      sim_move(&e->pos, &e->vel);
      if (e->age > 30 && e->pos.y < 150 && (e->age % sim_fire_interval(g, 105)) == 0) {
        for (int k = -2; k <= 2; k++) sim_aimed_shot(g, e->pos.x, e->pos.y + 8, 1.5f, (float)k * 0.22f);
        sim_sfx(g, SFX_ENEMY_SHOT);
      }
      break;
    case ET_SAUCER:
      sim_move(&e->pos, &e->vel);
      e->pos.x += ((e->age / 45) % 2 == 0) ? 1.9f : -1.9f;
      if (e->age > 25 && (e->age % sim_fire_interval(g, 80)) == 0 && e->pos.y < 170) sim_aimed_shot(g, e->pos.x, e->pos.y, 1.9f, 0);
      break;
    default: break;
  }
  if (e->pos.y > SCREEN_H + 24 || e->pos.y < -60 || e->pos.x < -40 || e->pos.x > SCREEN_W + 40) {
    if (e->age > 4) POOL_FREE(g->enemies, e);
  }
}

void enemy_update_all(Game* g) {
  POOL_EACH(Enemy, e, g->enemies) update_enemy(g, e);
}
