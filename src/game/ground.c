#include "game/sim.h"

bool ground_is_dino(GroundType t) { return t == GT_RAPTOR || t == GT_REX || t == GT_TRIKE || t == GT_SPINO || t == GT_BRONTO; }

void ground_half_size(GroundType t, float* hw, float* hh) {
  if (t == GT_BRONTO) { *hw = 24; *hh = 26; return; }
  if (t == GT_SPINO) { *hw = 17; *hh = 21; return; }
  if (t == GT_TRIKE) { *hw = 18; *hh = 18; return; }
  if (t == GT_FORT) { *hw = 15; *hh = 15; return; }
  if (t == GT_REX) { *hw = 14; *hh = 14; return; }
  if (t == GT_RAPTOR) { *hw = 8; *hh = 8; return; }
  *hw = 7;
  *hh = 7;
}

int ground_score(GroundType t) {
  static const int s[GT_COUNT] = {200, 300, 300, 250, 2000, 150, 300, 1500, 2500, 3000, 5000};
  return s[t];
}

Ground* ground_spawn(Game* g, GroundType type, float wx, float wy, float p0) {
  static const int hp[GT_COUNT] = {2, 6, 3, 3, 24, 2, 2, 10, 24, 30, 52};
  Ground* e = POOL_NEW(Ground, g->ground);
  if (!e) return NULL;
  e->type = type;
  e->pos = (Vec2){wx, wy};
  e->hp = hp[type] + ((type == GT_FORT || type == GT_BUNKER || type == GT_RAPTOR) ? 0 : g->difficulty / 4);
  e->p0 = p0;
  e->timer = 40 + (int)(wx * 3) % 50;
  if (type == GT_TANK) e->p0 = (p0 != 0) ? p0 : 1.0f;
  return e;
}

void ground_hit(Game* g, Ground* e, int dmg) {
  if (e->hp <= 0) return;
  e->hp -= dmg;
  e->flash = 3;
  float sy = sim_ground_sy(g, e->pos.y);
  if (e->hp > 0) { sim_sfx(g, SFX_HIT); return; }
  int pts = sim_kill_points(g, ground_score(e->type));
  sim_add_score(g, pts);
  g->kills++;
  sim_fx_score(g, e->pos.x, sy - 6, pts);
  sim_sfx(g, SFX_GROUND_DIE);
  sim_charge_gauge(g, e->type == GT_BUNKER ? 8 : 6);
  if (e->type == GT_FORT) {
    sim_fx(g, FX_EXPL_L, e->pos.x, sy, true);
    sim_sfx(g, SFX_EXPLODE_BIG);
  } else if (ground_is_dino(e->type)) {
    /* the creature collapses into a skeleton: the Ground entry stays as bones (hp <= 0), scrolling with the terrain */
    bool big = e->type != GT_RAPTOR;
    sim_fx(g, big ? FX_EXPL_L : FX_EXPL_S, e->pos.x, sy, true);
    if (e->type == GT_BRONTO) {
      sim_fx(g, FX_EXPL_L, e->pos.x - 14, sy + 10, true);
      sim_fx(g, FX_EXPL_L, e->pos.x + 14, sy - 10, true);
      sim_sfx(g, SFX_EXPLODE_BIG);
    }
    sim_sfx(g, SFX_DINO_DIE);
  } else {
    sim_fx(g, FX_EXPL_G, e->pos.x, sy, true);
  }
  if (e->type == GT_BUNKER) {
    int n = sim_next_missing_component(g);
    sim_pickup(g, n, e->pos.x, sy);
    sim_sfx(g, SFX_POD_APPEAR);
  } else if (e->type == GT_TRIKE || e->type == GT_SPINO || e->type == GT_BRONTO) {
    sim_weapon_drop(g, e->pos.x, sy); /* the big ones carry a weapon capsule */
    if (e->type == GT_BRONTO) sim_pickup(g, 0, e->pos.x + 12, sy);
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

/* Dinosaurs stomp across the ground and never wade into water (they turn away from it). p0 = heading (0 = down the screen).
   raptor: hunts the player's column. rex: ambles and spits fireballs. trike: stalks, winds up, then charges in a straight line.
   spino: stalks and spits aimed volleys. bronto: a slow giant whose stomp sends out a ring of shock-orbs. */
static void dino_walk(Game* g, Ground* e, float sy, bool onscreen) {
  if (!(sy > -12)) return;
  GroundType t = e->type;
  float desired;
  if (t == GT_REX) desired = 0.7f * sinf((float)e->age * 0.021f + e->pos.x);
  else if (t == GT_BRONTO) desired = 0.5f * sinf((float)e->age * 0.012f + e->pos.x);
  else if (g->player.state == PS_ALIVE) desired = atan2f(g->player.pos.x - e->pos.x, g->player.pos.y - sy);
  else desired = 0;
  float turn = 0.035f, speed = 0.95f;
  switch (t) {
    case GT_REX: turn = 0.015f; speed = 0.42f; break;
    case GT_TRIKE: turn = 0.02f; speed = 0.55f; break;
    case GT_SPINO: turn = 0.022f; speed = 0.6f; break;
    case GT_BRONTO: turn = 0.008f; speed = 0.28f; break;
    default: break;
  }
  /* the triceratops: state 0 stalk, 1 wind up (shakes, no movement), 2 charge (fast, no turning), 3 recover */
  if (t == GT_TRIKE && onscreen) {
    float dx = fabsf(g->player.pos.x - e->pos.x);
    if (e->state == 0 && e->age > 60 && dx < 60 && sy > 20 && sy < g->player.pos.y - 30) { e->state = 1; e->timer = 36; }
    else if (e->state == 1) {
      speed = 0;
      if (--e->timer <= 0) {
        e->state = 2; e->timer = 70;
        for (int k = -2; k <= 2; k++) sim_aimed_shot(g, e->pos.x, sy + 8, 1.6f, (float)k * 0.25f);
        sim_sfx(g, SFX_DINO_SPIT);
      }
    } else if (e->state == 2) {
      speed = 2.3f; turn = 0;
      if (--e->timer <= 0) { e->state = 3; e->timer = 60; }
    } else if (e->state == 3) {
      speed = 0.2f;
      if (--e->timer <= 0) e->state = 0;
    }
  }
  float diff = wrap_angle(desired - e->p0);
  e->p0 += diff > turn ? turn : (diff < -turn ? -turn : diff);
  speed += 0.03f * (float)g->difficulty;
  float nx = e->pos.x + sinf(e->p0) * speed;
  float nwy = e->pos.y - cosf(e->p0) * speed; /* world y grows upward, screen y downward */
  float margin = t == GT_BRONTO ? 26.0f : 14.0f;
  if (nx < margin || nx > SCREEN_W - margin || is_liquid_at(g, nx, nwy)) {
    e->p0 = wrap_angle(e->p0 + (e->pos.x < SCREEN_W / 2 ? -0.5f : 0.5f));
    if (t == GT_TRIKE && e->state == 2) { e->state = 3; e->timer = 60; }
  } else {
    e->pos.x = nx;
    e->pos.y = nwy;
  }
  if (!onscreen) return;
  if (t == GT_REX && --e->timer <= 0) {
    e->timer = sim_fire_interval(g, 150);
    sim_aimed_shot(g, e->pos.x, sy + 4, 1.6f, -0.18f);
    sim_aimed_shot(g, e->pos.x, sy + 4, 1.6f, 0.18f);
    sim_sfx(g, SFX_DINO_SPIT);
  } else if (t == GT_SPINO && --e->timer <= 0) {
    e->timer = sim_fire_interval(g, 110);
    for (int k = -2; k <= 2; k++) sim_aimed_shot(g, e->pos.x, sy + 12, 1.7f, (float)k * 0.22f);
    sim_sfx(g, SFX_DINO_SPIT);
  } else if (t == GT_BRONTO && --e->timer <= 0) {
    e->timer = sim_fire_interval(g, 170);
    int n = 14;
    float s = sim_bullet_speed(g, 1.3f), phase = (float)e->age * 0.1f;
    for (int k = 0; k < n; k++) {
      float a = phase + 6.2831853f * (float)k / (float)n;
      sim_ebullet(g, EB_ORB, e->pos.x, sy, sinf(a) * s, cosf(a) * s);
    }
    g->shake = g->shake > 6 ? g->shake : 6;
    sim_sfx(g, SFX_DINO_SPIT);
  }
}

static void update_ground(Game* g, Ground* e) {
  e->age++;
  if (e->flash > 0) e->flash--;
  float sy = sim_ground_sy(g, e->pos.y);
  if (sy > SCREEN_H + 32) { POOL_FREE(g->ground, e); return; }
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
    case GT_REX:
    case GT_TRIKE:
    case GT_SPINO:
    case GT_BRONTO: dino_walk(g, e, sy, onscreen); break;
    case GT_RADAR:
    default: break;
  }
}

void ground_update_all(Game* g) {
  POOL_EACH(Ground, e, g->ground) update_ground(g, e);
}
