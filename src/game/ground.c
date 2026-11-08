#include "game/sim.h"

void ground_half_size(GroundType t, float* hw, float* hh) {
  if (t == GT_FORT) { *hw = 15; *hh = 15; return; }
  if (t == GT_REX) { *hw = 14; *hh = 14; return; }
  if (t == GT_RAPTOR) { *hw = 8; *hh = 8; return; }
  *hw = 7;
  *hh = 7;
}

int ground_score(GroundType t) {
  static const int s[GT_COUNT] = {200, 300, 300, 250, 2000, 150, 300, 1500};
  return s[t];
}

Ground* ground_spawn(Game* g, GroundType type, float wx, float wy, float p0) {
  static const int hp[GT_COUNT] = {2, 6, 3, 3, 24, 2, 2, 10};
  for (int i = 0; i < MAX_GROUND; i++) {
    Ground* e = &g->ground[i];
    if (e->active) continue;
    *e = (Ground){0};
    e->active = true;
    e->type = type;
    e->pos = (Vec2){wx, wy};
    e->hp = hp[type] + ((type == GT_FORT || type == GT_BUNKER || type == GT_RAPTOR) ? 0 : g->difficulty / 4);
    e->p0 = p0;
    e->timer = 40 + (int)(wx * 3) % 50;
    if (type == GT_TANK) e->p0 = (p0 != 0) ? p0 : 1.0f;
    return e;
  }
  return NULL;
}

void ground_hit(Game* g, Ground* e, int dmg) {
  if (!e->active || e->hp <= 0) return;
  e->hp -= dmg;
  e->flash = 3;
  float sy = sim_ground_sy(g, e->pos.y);
  if (e->hp > 0) { sim_sfx(g, SFX_HIT); return; }
  int pts = ground_score(e->type);
  sim_add_score(g, pts);
  g->kills++;
  sim_fx_score(g, e->pos.x, sy - 6, pts);
  sim_sfx(g, SFX_GROUND_DIE);
  sim_charge_gauge(g, e->type == GT_BUNKER ? 8 : 6);
  if (e->type == GT_FORT) {
    sim_fx(g, FX_EXPL_L, e->pos.x, sy, true);
    sim_sfx(g, SFX_EXPLODE_BIG);
  } else if (e->type == GT_RAPTOR || e->type == GT_REX) {
    /* the creature collapses into a skeleton: the Ground entry stays as bones (hp <= 0), scrolling with the terrain */
    sim_fx(g, e->type == GT_REX ? FX_EXPL_L : FX_EXPL_S, e->pos.x, sy, true);
    sim_sfx(g, SFX_DINO_DIE);
  } else {
    sim_fx(g, FX_EXPL_G, e->pos.x, sy, true);
  }
  if (e->type == GT_BUNKER) {
    int n = sim_next_missing_component(g);
    sim_pickup(g, n, e->pos.x, sy);
    sim_sfx(g, SFX_POD_APPEAR);
  } else if (e->type == GT_RADAR || e->type == GT_FORT || e->type == GT_REX) {
    sim_pickup(g, 0, e->pos.x, sy);
  }
}

static int aim_frame(const Game* g, float x, float y) {
  const Player* p = &g->player;
  float dx = p->pos.x - x, dy = p->pos.y - y;
  float a = atan2f(dx, -dy); /* 0 = up, clockwise positive */
  int idx = (int)floorf(a / 0.78539816f + 0.5f);
  return ((idx % 8) + 8) % 8;
}

static float wrap_angle(float a) {
  while (a > 3.14159265f) a -= 6.2831853f;
  while (a < -3.14159265f) a += 6.2831853f;
  return a;
}

static bool is_liquid_at(const Game* g, float wx, float wy) {
  int col = (int)(wx / TILE), row = (int)(wy / TILE);
  int local = level_tile_sprite_local(level_tile(g, row, col));
  return local >= 17 && local <= 32;
}

/* Dinosaurs stomp across the ground: raptors hunt the player's column, rexes amble down and spit fireballs.
   They never wade into water; they turn away from it. p0 = heading (0 = down the screen). */
static void dino_walk(Game* g, Ground* e, float sy, bool onscreen) {
  if (!(sy > -12)) return;
  bool rex = e->type == GT_REX;
  float desired;
  if (rex) desired = 0.7f * sinf((float)e->age * 0.021f + e->pos.x);
  else if (g->player.state == PS_ALIVE) desired = atan2f(g->player.pos.x - e->pos.x, g->player.pos.y - sy);
  else desired = 0;
  float diff = wrap_angle(desired - e->p0);
  float turn = rex ? 0.015f : 0.035f;
  e->p0 += diff > turn ? turn : (diff < -turn ? -turn : diff);
  float speed = (rex ? 0.42f : 0.95f) + 0.03f * (float)g->difficulty;
  float nx = e->pos.x + sinf(e->p0) * speed;
  float nwy = e->pos.y - cosf(e->p0) * speed; /* world y grows upward, screen y downward */
  if (nx < 14 || nx > SCREEN_W - 14 || is_liquid_at(g, nx, nwy)) {
    e->p0 = wrap_angle(e->p0 + (e->pos.x < SCREEN_W / 2 ? -0.5f : 0.5f));
  } else {
    e->pos.x = nx;
    e->pos.y = nwy;
  }
  if (rex && onscreen && --e->timer <= 0) {
    e->timer = sim_fire_interval(g, 150);
    sim_aimed_shot(g, e->pos.x, sy + 4, 1.6f, -0.18f);
    sim_aimed_shot(g, e->pos.x, sy + 4, 1.6f, 0.18f);
    sim_sfx(g, SFX_DINO_SPIT);
  }
}

static void update_ground(Game* g, Ground* e) {
  e->age++;
  if (e->flash > 0) e->flash--;
  float sy = sim_ground_sy(g, e->pos.y);
  if (sy > SCREEN_H + 32) { e->active = false; return; }
  if (e->hp <= 0) return; /* rubble */
  bool onscreen = sy > 10 && sy < 185;
  switch (e->type) {
    case GT_TURRET:
      e->aim = aim_frame(g, e->pos.x, sy);
      if (onscreen && --e->timer <= 0) {
        e->timer = sim_fire_interval(g, 110);
        sim_aimed_shot(g, e->pos.x, sy, 1.7f, 0);
      }
      break;
    case GT_BUNKER:
      e->state = ((e->age / 24) % 2);
      break;
    case GT_TANK:
      e->pos.x += e->p0 * 0.55f;
      if (e->pos.x < 24) e->p0 = 1.0f;
      if (e->pos.x > SCREEN_W - 24) e->p0 = -1.0f;
      if (onscreen && --e->timer <= 0) {
        e->timer = sim_fire_interval(g, 140);
        sim_aimed_shot(g, e->pos.x, sy, 1.6f, 0);
      }
      break;
    case GT_SILO:
      if (e->state == 0 && onscreen && --e->timer <= 0) { e->state = 1; e->timer = 0; }
      if (e->state >= 1) {
        e->timer++;
        e->state = 1 + e->timer / 5;
        if (e->state > 3) e->state = 3;
        if (e->timer == 16) {
          float vx = (g->player.pos.x - e->pos.x) * 0.006f;
          EBullet* b = sim_ebullet(g, EB_MISSILE, e->pos.x, sy + 4, vx, 0.6f);
          if (b) b->accel = 0.035f;
          sim_sfx(g, SFX_ENEMY_SHOT);
        }
        if (e->timer > 34) { e->state = 0; e->timer = sim_fire_interval(g, 170); }
      }
      break;
    case GT_FORT:
      if (onscreen && --e->timer <= 0) {
        e->timer = sim_fire_interval(g, 120);
        e->state = 12;
        for (int k = -2; k <= 2; k++) sim_aimed_shot(g, e->pos.x, sy, 1.5f, (float)k * 0.2f);
        sim_sfx(g, SFX_ENEMY_SHOT);
      }
      if (e->state > 0) e->state--;
      break;
    case GT_RAPTOR:
    case GT_REX: dino_walk(g, e, sy, onscreen); break;
    case GT_RADAR:
    default: break;
  }
}

void ground_update_all(Game* g) {
  for (int i = 0; i < MAX_GROUND; i++)
    if (g->ground[i].active) update_ground(g, &g->ground[i]);
}
