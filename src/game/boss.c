#include <string.h>

#include "game/sim.h"

#define BOSS_ENTER_TICKS 130
#define BOSS_REST_Y 58.0f
#define BOSS_DEATH_TICKS 130
#define BEAM_WARN 45
#define BEAM_ON 50

void boss_core_pos(const Boss* b, float* x, float* y) {
  *x = b->pos.x;
  *y = b->pos.y + 14;
}

void boss_gun_pos(const Boss* b, int i, float* x, float* y) {
  *x = b->pos.x + (i == 0 ? -28.0f : 28.0f);
  *y = b->pos.y + 8;
}

void boss_start(Game* g) {
  Boss* b = &g->boss;
  memset(b, 0, sizeof *b);
  b->active = true;
  b->id = g->stage;
  b->pos = (Vec2){128, -64};
  b->entering = BOSS_ENTER_TICKS;
  b->gun_max = 26 + 9 * g->stage + 4 * g->loop;
  b->gun_hp[0] = b->gun_hp[1] = b->gun_max;
  b->core_max = 90 + 34 * g->stage + 16 * g->loop;
  b->core_hp = b->core_max;
}

static void ring(Game* g, float x, float y, int n, float speed, float offset) {
  for (int i = 0; i < n; i++) {
    float a = offset + 6.2831853f * (float)i / (float)n;
    float s = sim_bullet_speed(g, speed);
    sim_ebullet(g, EB_ORB, x, y, sinf(a) * s, cosf(a) * s);
  }
}

static void stream(Game* g, float x, float y, float angle, float speed) {
  float s = sim_bullet_speed(g, speed);
  sim_ebullet(g, EB_ORB, x, y, sinf(angle) * s, cosf(angle) * s);
}

static void add_beam(Boss* b, float x) {
  for (int i = 0; i < MAX_BEAMS; i++) {
    if (b->beams[i].warn <= 0 && b->beams[i].on <= 0) {
      b->beams[i].x = x;
      b->beams[i].warn = BEAM_WARN;
      b->beams[i].on = BEAM_ON;
      return;
    }
  }
}

static void attack(Game* g, Boss* b) {
  int t = b->t;
  float cx, cy;
  boss_core_pos(b, &cx, &cy);
  float gx[2], gy[2];
  for (int i = 0; i < 2; i++) boss_gun_pos(b, i, &gx[i], &gy[i]);
  const Player* pl = &g->player;
  switch (b->id) {
    case 0: /* Gargantua: aimed bursts, then rings, then spiral */
      if (b->phase == 0) {
        for (int i = 0; i < 2; i++)
          if (b->gun_hp[i] > 0 && (t + i * 35) % sim_fire_interval(g, 75) == 0)
            for (int k = -1; k <= 1; k++) sim_aimed_shot(g, gx[i], gy[i], 1.9f, (float)k * 0.2f);
      } else {
        if (t % sim_fire_interval(g, 100) == 0) ring(g, cx, cy, 12, 1.4f, (float)t * 0.01f);
        if (t % sim_fire_interval(g, 70) == 0) { sim_aimed_shot(g, cx, cy, 2.0f, -0.1f); sim_aimed_shot(g, cx, cy, 2.0f, 0.1f); }
        if (b->phase == 2 && t % 6 == 0) { b->spiral += 0.37f; stream(g, cx, cy, b->spiral, 1.6f); }
      }
      break;
    case 1: /* Hydra: fans, sweeping streams and beams */
      if (b->phase == 0) {
        for (int i = 0; i < 2; i++)
          if (b->gun_hp[i] > 0 && (t + i * 40) % sim_fire_interval(g, 85) == 0)
            for (int k = -2; k <= 2; k++) sim_aimed_shot(g, gx[i], gy[i], 1.7f, (float)k * 0.24f);
      } else {
        if (t % 5 == 0) stream(g, cx, cy, sinf((float)t * 0.05f) * 0.9f, 1.8f);
        if (t % sim_fire_interval(g, 170) == 0) add_beam(b, pl->pos.x);
        if (b->phase == 2) {
          if (t % sim_fire_interval(g, 110) == 0) ring(g, cx, cy, 14, 1.5f, 0);
          if (t % sim_fire_interval(g, 170) == 85) { add_beam(b, pl->pos.x - 44); add_beam(b, pl->pos.x + 44); }
        }
      }
      break;
    default: /* Mother Core: rapid streams, spirals, drone summons, triple beams */
      if (b->phase == 0) {
        for (int i = 0; i < 2; i++)
          if (b->gun_hp[i] > 0 && (t + i * 7) % sim_fire_interval(g, 16) == 0) sim_aimed_shot(g, gx[i], gy[i], 2.1f, 0);
        if (t % 140 == 70) sim_aimed_shot(g, cx, cy, 1.6f, 0);
      } else {
        if (t % 5 == 0) { b->spiral += 0.3f; stream(g, cx, cy, b->spiral, 1.7f); stream(g, cx, cy, b->spiral + 3.14159f, 1.7f); }
        if (t % 220 == 100) { enemy_spawn(g, ET_DRONE, cx - 30, cy + 20, 0, 1); enemy_spawn(g, ET_DRONE, cx + 30, cy + 20, 2, 1); }
        if (b->phase == 2) {
          if (t % 6 == 3) stream(g, cx, cy, b->spiral * -1.3f + 1.0f, 1.6f);
          if (t % sim_fire_interval(g, 200) == 0) { add_beam(b, pl->pos.x); add_beam(b, pl->pos.x - 60); add_beam(b, pl->pos.x + 60); }
          if (t % sim_fire_interval(g, 120) == 60) ring(g, cx, cy, 16, 1.4f, 0.2f);
        }
      }
      break;
  }
}

static void movement(Boss* b) {
  float t = (float)b->t;
  switch (b->id) {
    case 0: b->pos.x = 128 + 50 * sinf(t * 0.018f); b->pos.y = BOSS_REST_Y; break;
    case 1: b->pos.x = 128 + 68 * sinf(t * 0.022f); b->pos.y = BOSS_REST_Y + 8 * sinf(t * 0.03f); break;
    default: b->pos.x = 128 + 42 * sinf(t * 0.015f); b->pos.y = BOSS_REST_Y - 4 + 10 * sinf(t * 0.04f); break;
  }
}

static void boss_explode_step(Game* g, Boss* b) {
  b->dying++;
  if (b->dying % 5 == 0) {
    float ox = (float)rng_range(&g->rng, -44, 44), oy = (float)rng_range(&g->rng, -24, 24);
    sim_fx(g, b->dying % 10 == 0 ? FX_EXPL_L : FX_EXPL_S, b->pos.x + ox, b->pos.y + oy, false);
    sim_sfx(g, SFX_EXPLODE_BIG);
    g->shake = 6;
  }
  if (b->dying >= BOSS_DEATH_TICKS) {
    for (int i = 0; i < 5; i++) sim_fx(g, FX_EXPL_L, b->pos.x + (float)(i - 2) * 22, b->pos.y + (float)((i % 2) * 10 - 5), false);
    sim_sfx(g, SFX_BOSS_DIE);
    g->screen_flash = 1.0f;
    sim_pickup(g, 0, b->pos.x - 20, b->pos.y + 20);
    sim_pickup(g, 0, b->pos.x + 20, b->pos.y + 20);
    sim_add_score(g, 3000 * (g->stage + 1));
    sim_fx_score(g, b->pos.x, b->pos.y, 3000 * (g->stage + 1));
    b->active = false;
  }
}

void boss_update(Game* g) {
  Boss* b = &g->boss;
  if (!b->active) return;
  b->t++;
  for (int i = 0; i < 2; i++) if (b->flash_gun[i] > 0) b->flash_gun[i]--;
  if (b->flash_core > 0) b->flash_core--;
  if (b->flash_body > 0) b->flash_body--;
  if (b->dying) { boss_explode_step(g, b); return; }
  if (b->entering > 0) {
    b->entering--;
    float u = 1.0f - (float)b->entering / (float)BOSS_ENTER_TICKS;
    b->pos.y = -64 + (BOSS_REST_Y + 64) * u * u * (3 - 2 * u);
    return;
  }
  movement(b);
  attack(g, b);
  for (int i = 0; i < MAX_BEAMS; i++) {
    Beam* m = &b->beams[i];
    if (m->warn > 0) {
      m->warn--;
      if (m->warn == 0) sim_sfx(g, SFX_BEAM);
    } else if (m->on > 0) {
      m->on--;
      float y0 = b->pos.y + 30;
      player_hit_rect(g, m->x, (y0 + SCREEN_H) * 0.5f, 4, (SCREEN_H - y0) * 0.5f);
    }
  }
  /* touching the armoured hull is lethal */
  player_hit_rect(g, b->pos.x, b->pos.y, 38, 20);
}

bool boss_hurts_rect(const Game* g, float x, float y, float hw, float hh) {
  const Boss* b = &g->boss;
  for (int i = 0; i < MAX_BEAMS; i++) {
    if (b->beams[i].on > 0 && b->beams[i].warn <= 0) {
      float y0 = b->pos.y + 30;
      if (sim_overlap(x, y, hw, hh, b->beams[i].x, (y0 + SCREEN_H) * 0.5f, 4, (SCREEN_H - y0) * 0.5f)) return true;
    }
  }
  return false;
}

static void damage_core(Game* g, Boss* b, int dmg) {
  b->core_hp -= dmg;
  b->flash_core = 3;
  sim_sfx(g, SFX_BOSS_HIT);
  if (b->phase == 1 && b->core_hp * 10 <= b->core_max * 4) {
    b->phase = 2;
    sim_sfx(g, SFX_BOSS_WARNING);
    sim_kill_all_enemy_bullets_to_fx(g);
    g->shake = 12;
  }
  if (b->core_hp <= 0) {
    b->core_hp = 0;
    b->dying = 1;
    for (int i = 0; i < MAX_BEAMS; i++) b->beams[i].warn = b->beams[i].on = 0;
    sim_kill_all_enemy_bullets_to_fx(g);
  }
}

bool boss_hit_test(Game* g, PBullet* bl) {
  Boss* b = &g->boss;
  if (!b->active || b->dying || b->entering > 0) return false;
  float bx = bl->pos.x, by = bl->pos.y;
  float bw = bl->kind == PB_PHOENIX ? 8 : 3;
  float bh = bl->kind == PB_PHOENIX ? 11 : 6;
  for (int i = 0; i < 2; i++) {
    if (b->gun_hp[i] <= 0) continue;
    float gx, gy;
    boss_gun_pos(b, i, &gx, &gy);
    /* a gun's hit column runs from its top down to the hull's lower edge, so shots from below reach it */
    float col_top = gy - 8, col_bot = b->pos.y + 28;
    if (sim_overlap(bx, by, bw, bh, gx, (col_top + col_bot) * 0.5f, 8, (col_bot - col_top) * 0.5f)) {
      b->gun_hp[i] -= bl->dmg;
      b->flash_gun[i] = 3;
      sim_sfx(g, SFX_BOSS_HIT);
      sim_fx(g, FX_SPARK, bx, by, false);
      if (b->gun_hp[i] <= 0) {
        b->gun_hp[i] = 0;
        sim_fx(g, FX_EXPL_L, gx, gy, false);
        sim_sfx(g, SFX_EXPLODE_BIG);
        sim_add_score(g, 1000);
        sim_fx_score(g, gx, gy - 10, 1000);
        sim_charge_gauge(g, 15);
        g->shake = 10;
        if (b->gun_hp[0] <= 0 && b->gun_hp[1] <= 0) {
          b->phase = 1;
          sim_kill_all_enemy_bullets_to_fx(g);
          sim_sfx(g, SFX_BOSS_WARNING);
        }
      }
      return true;
    }
  }
  float cx, cy;
  boss_core_pos(b, &cx, &cy);
  float core_top = cy - 8, core_bot = b->pos.y + 28;
  if (sim_overlap(bx, by, bw, bh, cx, (core_top + core_bot) * 0.5f, 8, (core_bot - core_top) * 0.5f)) {
    if (b->phase >= 1) {
      damage_core(g, b, bl->dmg);
      sim_fx(g, FX_SPARK, bx, by, false);
    } else {
      sim_fx(g, FX_SPARK, bx, by, false); /* shielded: deflected */
      b->flash_body = 2;
    }
    return true;
  }
  if (sim_overlap(bx, by, bw, bh, b->pos.x, b->pos.y, 46, 26)) {
    sim_fx(g, FX_SPARK, bx, by + 2, false); /* armour plating absorbs the shot */
    b->flash_body = 2;
    return true;
  }
  return false;
}
