#include <string.h>

#include "game/sim.h"

static const Vec2 kSlots[NUM_COMPONENTS] = {{-12, 2}, {12, 2}, {0, -16}, {0, 15}, {-22, 6}, {22, 6}, {-10, -12}, {10, -12}};
static const float kCompHalfW[NUM_COMPONENTS] = {5, 5, 5, 7, 5, 5, 3, 3};
static const float kCompHalfH[NUM_COMPONENTS] = {7, 7, 5, 4, 6, 6, 4, 4};
#define PI_F 3.14159265f

Vec2 player_slot_offset(int idx) { return kSlots[idx]; }

static float ease(float u) {
  if (u < 0) u = 0;
  if (u > 1) u = 1;
  return u * u * (3.0f - 2.0f * u);
}

static Vec2 lerp(Vec2 a, Vec2 b, float u) { return (Vec2){a.x + (b.x - a.x) * u, a.y + (b.y - a.y) * u}; }

Vec2 formation_offset(int idx, int rank, int count, int state, int t) {
  Vec2 slot = kSlots[idx];
  if (count < 1) count = 1;
  float a0 = -PI_F / 2.0f + 2.0f * PI_F * (float)rank / (float)count;
  float a = a0 + FORM_ORBIT_SPEED * (float)t;
  /* with more than four modules two rings counter-rotate: even ranks inside, odd ranks outside */
  float radius = FORM_ORBIT_RADIUS;
  if (count > 4) {
    radius = (rank % 2 == 0) ? FORM_ORBIT_RADIUS_INNER : FORM_ORBIT_RADIUS_OUTER;
    if (rank % 2 == 1) a = a0 - FORM_ORBIT_SPEED * (float)t;
  }
  Vec2 orbit = {radius * cosf(a), radius * sinf(a)};
  switch (state) {
    case FORM_SCATTER: return lerp(slot, orbit, ease((float)t / (float)FORM_SCATTER_TICKS));
    case FORM_HOLD: return orbit;
    case FORM_REUNITE: return lerp(orbit, slot, ease((float)(t - FORM_SCATTER_TICKS - FORM_HOLD_TICKS) / (float)FORM_REUNITE_TICKS));
    default: return slot;
  }
}

bool player_comp_docked(const Game* g, int idx) { return g->player.comp[idx] && g->player.dock_t[idx] >= DOCK_TICKS; }

int player_component_count(const Game* g) {
  int n = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++) n += g->player.comp[i] ? 1 : 0;
  return n;
}

int player_rank_of(const Game* g, int idx) {
  int r = 0;
  for (int i = 0; i < idx; i++) r += g->player.comp[i] ? 1 : 0;
  return r;
}

Vec2 player_comp_offset(const Game* g, int idx) {
  const Player* p = &g->player;
  if (p->form_state != FORM_NONE) return formation_offset(idx, player_rank_of(g, idx), player_component_count(g), p->form_state, p->form_t);
  if (p->dock_t[idx] < DOCK_TICKS) return lerp(p->dock_from[idx], kSlots[idx], ease((float)p->dock_t[idx] / (float)DOCK_TICKS));
  return kSlots[idx];
}

bool player_component_rect(const Game* g, int idx, float* x, float* y, float* hw, float* hh) {
  if (!g->player.comp[idx]) return false;
  Vec2 o = player_comp_offset(g, idx);
  *x = g->player.pos.x + o.x;
  *y = g->player.pos.y + o.y;
  *hw = kCompHalfW[idx];
  *hh = kCompHalfH[idx];
  return true;
}

void player_hurtbox_core(const Game* g, float* x, float* y, float* hw, float* hh) {
  *x = g->player.pos.x;
  *y = g->player.pos.y + 1;
  *hw = 3.0f;
  *hh = 6.0f;
}

bool player_is_phoenix(const Game* g) {
  if (g->player.form_state == FORM_NONE) return false;
  for (int i = 0; i < NUM_COMPONENTS; i++)
    if (!g->player.comp[i]) return false;
  return true;
}

bool player_invulnerable(const Game* g) {
  return g->god_mode || g->player.invuln > 0 || g->player.form_state != FORM_NONE || g->player.state != PS_ALIVE;
}

float player_speed(const Game* g) { return PLAYER_SPEED * (player_comp_docked(g, 3) ? PLAYER_SPEED_C4_BONUS : 1.0f); }

void player_reset(Game* g, bool full) {
  Player* p = &g->player;
  p->pos = (Vec2){PLAYER_SPAWN_X, PLAYER_SPAWN_Y};
  p->state = PS_ALIVE;
  p->invuln = full ? 120 : RESPAWN_INVULN;
  p->shot_cd = 0;
  p->fire_duty = 0.5f;
  p->form_state = FORM_NONE;
  p->form_t = 0;
  p->form_cd = 0;
  p->respawn_timer = 0;
  p->bank = 0;
  p->hit_flash = 0;
  memset(p->comp, 0, sizeof p->comp);
  memset(p->dock_t, 0, sizeof p->dock_t);
  p->gauge = 0;
}

/* ---------------- damage ---------------- */

void player_lose_component(Game* g, int idx) {
  Player* p = &g->player;
  if (!p->comp[idx]) return;
  Vec2 o = player_comp_offset(g, idx);
  sim_fx(g, FX_EXPL_S, p->pos.x + o.x, p->pos.y + o.y, false);
  p->comp[idx] = false;
  p->dock_t[idx] = 0;
  p->invuln = COMP_HIT_INVULN;
  p->hit_flash = 12;
  g->shake = 8;
  sim_sfx(g, SFX_COMPONENT_LOST);
}

void player_die(Game* g) {
  Player* p = &g->player;
  if (p->state != PS_ALIVE) return;
  sim_fx(g, FX_DEATH, p->pos.x, p->pos.y, false);
  for (int i = 0; i < NUM_COMPONENTS; i++) {
    if (p->comp[i]) {
      Vec2 o = player_comp_offset(g, i);
      sim_fx(g, FX_EXPL_S, p->pos.x + o.x, p->pos.y + o.y, false);
    }
  }
  memset(p->comp, 0, sizeof p->comp);
  p->weapon = WT_VULCAN; /* a lost ship loses its special weapon too */
  p->state = PS_DEAD;
  p->lives--;
  p->respawn_timer = RESPAWN_DELAY;
  p->form_state = FORM_NONE;
  p->gauge = 0;
  g->shake = 20;
  sim_sfx(g, SFX_PLAYER_DIE);
}

static bool circle_rect(float cx, float cy, float r, float rx, float ry, float hw, float hh) {
  return sim_overlap(cx, cy, r, r, rx, ry, hw, hh);
}

bool player_try_hit(Game* g, float x, float y, float radius) {
  Player* p = &g->player;
  if (player_invulnerable(g)) return false;
  float cx, cy, chw, chh;
  player_hurtbox_core(g, &cx, &cy, &chw, &chh);
  if (circle_rect(x, y, radius, cx, cy, chw, chh)) {
    player_die(g);
    return true;
  }
  for (int i = NUM_COMPONENTS - 1; i >= 0; i--) {
    float rx, ry, hw, hh;
    if (player_comp_docked(g, i) && player_component_rect(g, i, &rx, &ry, &hw, &hh) && circle_rect(x, y, radius, rx, ry, hw, hh)) {
      player_lose_component(g, i);
      return true;
    }
  }
  (void)p;
  return false;
}

bool player_hit_rect(Game* g, float x, float y, float hw, float hh) {
  if (player_invulnerable(g)) return false;
  float cx, cy, chw, chh;
  player_hurtbox_core(g, &cx, &cy, &chw, &chh);
  if (sim_overlap(x, y, hw, hh, cx, cy, chw, chh)) {
    player_die(g);
    return true;
  }
  for (int i = NUM_COMPONENTS - 1; i >= 0; i--) {
    float rx, ry, chw2, chh2;
    if (player_comp_docked(g, i) && player_component_rect(g, i, &rx, &ry, &chw2, &chh2) && sim_overlap(x, y, hw, hh, rx, ry, chw2, chh2)) {
      player_lose_component(g, i);
      return true;
    }
  }
  return false;
}

/* ---------------- docking ---------------- */

void player_dock(Game* g, int number, Vec2 from) {
  Player* p = &g->player;
  int idx = number - 1;
  p->comp[idx] = true;
  p->dock_t[idx] = 0;
  p->dock_from[idx] = from;
  g->stats_docks++;
  sim_add_score(g, 500);
  sim_fx_score(g, p->pos.x, p->pos.y - 14, 500);
  sim_fx(g, FX_DOCK, p->pos.x + kSlots[idx].x, p->pos.y + kSlots[idx].y, false);
  sim_charge_gauge(g, 25);
  sim_sfx(g, SFX_DOCK);
}

/* ---------------- shooting ---------------- */

static PBullet* new_pbullet(Game* g, PBulletKind kind, float x, float y, float vx, float vy, int dmg, bool pierce) {
  PBullet* b = POOL_NEW(PBullet, g->pbullets);
  if (!b) return NULL;
  b->kind = kind;
  b->pos = point(x, y);
  sim_set_velocity(&b->vel, vx, vy);
  b->dmg = dmg;
  b->pierce = pierce;
  b->age = 0;
  b->life = 0;
  b->ox = x;
  b->phase = 0;
  b->amp = 0;
  return b;
}

static int docked_count(const Game* g) {
  int n = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++) n += player_comp_docked(g, i) ? 1 : 0;
  return n;
}

/* every pair of docked modules is one weapon tier (0..4); the weapon TYPE decides what the tiers look like */
static void fire_vulcan(Game* g, int n, int tier) {
  Player* p = &g->player;
  float x = p->pos.x, y = p->pos.y;
  float my = y - 13;
  switch (tier) {
    case 0: /* needle */
      new_pbullet(g, PB_NORMAL, x, my, 0, -5.4f, 1, false);
      break;
    case 1: /* twin bolts */
      new_pbullet(g, PB_BOLT, x - 4, my, 0, -5.6f, 2, false);
      new_pbullet(g, PB_BOLT, x + 4, my, 0, -5.6f, 2, false);
      break;
    case 2: /* trident: a lance up the middle, bolts on either side */
      new_pbullet(g, PB_LANCE, x, my - 4, 0, -6.4f, 3, false);
      new_pbullet(g, PB_BOLT, x - 7, my + 2, -0.9f, -5.6f, 2, false);
      new_pbullet(g, PB_BOLT, x + 7, my + 2, 0.9f, -5.6f, 2, false);
      break;
    case 3: /* piercing lances in a fan */
      new_pbullet(g, PB_LANCE, x, my - 4, 0, -6.8f, 3, true);
      new_pbullet(g, PB_LANCE, x - 6, my, -0.8f, -6.6f, 3, true);
      new_pbullet(g, PB_LANCE, x + 6, my, 0.8f, -6.6f, 3, true);
      new_pbullet(g, PB_BOLT, x - 12, my + 4, -1.8f, -5.6f, 2, false);
      new_pbullet(g, PB_BOLT, x + 12, my + 4, 1.8f, -5.6f, 2, false);
      break;
    default: /* plasma: a piercing ball, flanked by lances and bolts */
      new_pbullet(g, PB_PLASMA, x, my - 6, 0, -5.8f, 6, true);
      new_pbullet(g, PB_LANCE, x - 9, my, -0.5f, -6.8f, 3, true);
      new_pbullet(g, PB_LANCE, x + 9, my, 0.5f, -6.8f, 3, true);
      new_pbullet(g, PB_BOLT, x - 15, my + 4, -1.8f, -5.6f, 2, false);
      new_pbullet(g, PB_BOLT, x + 15, my + 4, 1.8f, -5.6f, 2, false);
      break;
  }
  if (n > 0 && n % 2 == 0) { /* the second module of a pair adds escort shots */
    new_pbullet(g, PB_WIDE, x - 18, y - 4, -2.4f, -4.8f, 1, false);
    new_pbullet(g, PB_WIDE, x + 18, y - 4, 2.4f, -4.8f, 1, false);
  }
  sim_fx(g, FX_MUZZLE, x, y - 15, false);
  if (tier >= 2) sim_sfx(g, SFX_SHOT_HEAVY);
  else if (p->shot_alt % 2 == 0) sim_sfx(g, SFX_SHOT);
}

/* SPREAD: a fan of short-range pellets, wider and denser with every tier */
static void fire_spread(Game* g, int tier) {
  Player* p = &g->player;
  float x = p->pos.x, y = p->pos.y;
  int count = 3 + 2 * tier;
  for (int i = 0; i < count; i++) {
    float a = ((float)i - (float)(count - 1) * 0.5f) * 0.15f;
    PBullet* b = new_pbullet(g, PB_PELLET, x + sinf(a) * 6, y - 12 - cosf(a) * 2, sinf(a) * 5.0f, -cosf(a) * 5.0f, tier >= 2 ? 2 : 1, tier >= 4);
    if (b) b->life = 42 + 4 * tier;
  }
  sim_fx(g, FX_MUZZLE, x, y - 15, false);
  sim_sfx(g, SFX_SHOT);
}

/* LASER: fast thin piercing beams, side by side */
static void fire_laser(Game* g, int tier) {
  Player* p = &g->player;
  float x = p->pos.x, y = p->pos.y;
  static const int beams[5] = {1, 2, 3, 3, 4};
  int count = beams[tier];
  for (int i = 0; i < count; i++) {
    float off = ((float)i - (float)(count - 1) * 0.5f) * 6.0f;
    float tilt = (tier >= 4 && (i == 0 || i == count - 1)) ? off * 0.03f : 0.0f;
    new_pbullet(g, PB_LASER, x + off, y - 18, tilt, -9.0f, tier >= 3 ? 2 : 1, true);
  }
  sim_fx(g, FX_MUZZLE, x, y - 20, false);
  if (p->shot_alt % 3 == 0) sim_sfx(g, SFX_BEAM);
}

/* MISSILE: homing missiles that burst out sideways, then curve onto the nearest target */
static void fire_missile(Game* g, int tier) {
  Player* p = &g->player;
  float x = p->pos.x, y = p->pos.y;
  static const int counts[5] = {2, 2, 4, 4, 6};
  int count = counts[tier];
  for (int i = 0; i < count; i++) {
    float side = (i % 2 == 0) ? -1.0f : 1.0f;
    float rank = (float)(i / 2);
    PBullet* b = new_pbullet(g, PB_MISSILE, x + side * (8 + 3 * rank), y - 6, side * (1.2f + 0.5f * rank), -2.6f, tier >= 3 ? 3 : 2, false);
    if (b) b->life = 150;
  }
  sim_fx(g, FX_MUZZLE, x - 8, y - 8, false);
  sim_fx(g, FX_MUZZLE, x + 8, y - 8, false);
  sim_sfx(g, SFX_SHOT_HEAVY);
}

/* WAVE: piercing crescents that snake up the screen in counter-phase */
static void fire_wave(Game* g, int tier) {
  Player* p = &g->player;
  float x = p->pos.x, y = p->pos.y;
  static const int pairs[5] = {1, 1, 2, 2, 3};
  int np = pairs[tier];
  for (int j = 0; j < np; j++) {
    float lat = ((float)j - (float)(np - 1) * 0.5f) * 20.0f;
    for (int k = 0; k < 2; k++) {
      PBullet* b = new_pbullet(g, PB_WAVE, x + lat, y - 14, 0, -4.4f, tier >= 2 ? 2 : 1, true);
      if (b) {
        b->ox = x + lat;
        b->phase = k == 0 ? 0.0f : 3.14159265f;
        b->amp = 12.0f + 2.0f * (float)tier;
      }
    }
  }
  sim_fx(g, FX_MUZZLE, x, y - 15, false);
  if (p->shot_alt % 2 == 0) sim_sfx(g, SFX_SHOT);
}

static void fire_volley(Game* g) {
  Player* p = &g->player;
  float x = p->pos.x, y = p->pos.y;
  if (player_is_phoenix(g)) {
    new_pbullet(g, PB_PHOENIX, x, y - 18, 0, -5.5f, 4, true);
    sim_fx(g, FX_MUZZLE, x, y - 20, false);
    sim_sfx(g, SFX_SHOT_HEAVY);
    return;
  }
  bool detached = p->form_state != FORM_NONE;
  int n = detached ? 0 : docked_count(g);
  int tier = (n + 1) / 2;
  switch (p->weapon) {
    case WT_SPREAD: fire_spread(g, tier); break;
    case WT_LASER: fire_laser(g, tier); break;
    case WT_MISSILE: fire_missile(g, tier); break;
    case WT_WAVE: fire_wave(g, tier); break;
    default: fire_vulcan(g, n, tier); break;
  }
  p->shot_alt++;
}

int player_weapon_interval(const Game* g) {
  if (player_is_phoenix(g)) return 9;
  switch (g->player.weapon) {
    case WT_SPREAD: return 10;
    case WT_LASER: return 6;
    case WT_MISSILE: return 16;
    case WT_WAVE: return 10;
    default: return SHOT_INTERVAL;
  }
}

static void fire_formation_components(Game* g) {
  Player* p = &g->player;
  int rank = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++) {
    if (!p->comp[i]) continue;
    if (--p->comp_shot_cd[i] <= 0) {
      p->comp_shot_cd[i] = 11;
      Vec2 o = player_comp_offset(g, i);
      float len = sqrtf(o.x * o.x + o.y * o.y);
      if (len < 1) len = 1;
      float dx = o.x / len, dy = o.y / len; /* radial: pinwheel pattern */
      new_pbullet(g, PB_FORM, p->pos.x + o.x, p->pos.y + o.y, dx * 3.6f, dy * 3.6f, 1, false);
      /* every other shot also fires straight up for sustained forward damage */
      if ((p->form_t / 11 + rank) % 2 == 0) new_pbullet(g, PB_FORM, p->pos.x + o.x, p->pos.y + o.y - 6, 0, -4.6f, 1, false);
    }
    rank++;
  }
}

/* ---------------- formation attack ---------------- */

bool player_can_start_formation(const Game* g) {
  const Player* p = &g->player;
  if (p->state != PS_ALIVE || p->form_state != FORM_NONE || p->form_cd > 0) return false;
  if (p->gauge < GAUGE_MAX) return false;
  int docked = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++) docked += player_comp_docked(g, i) ? 1 : 0;
  return docked >= FORMATION_MIN_COMPONENTS && docked == player_component_count(g);
}

void player_start_formation(Game* g) {
  Player* p = &g->player;
  p->form_state = FORM_SCATTER;
  p->form_t = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++) p->comp_shot_cd[i] = 6 + 3 * i;
  g->stats_formations++;
  g->screen_flash = 0.7f;
  g->shake = 10;
  sim_sfx(g, SFX_FORMATION_START);
  for (int i = 0; i < NUM_COMPONENTS; i++)
    if (p->comp[i]) sim_fx(g, FX_DOCK, p->pos.x + kSlots[i].x, p->pos.y + kSlots[i].y, false);
}

static void formation_update(Game* g) {
  Player* p = &g->player;
  if (p->form_state == FORM_NONE) {
    if (p->form_cd > 0) p->form_cd--;
    return;
  }
  p->form_t++;
  int t = p->form_t;
  if (t < FORM_SCATTER_TICKS) p->form_state = FORM_SCATTER;
  else if (t < FORM_SCATTER_TICKS + FORM_HOLD_TICKS) p->form_state = FORM_HOLD;
  else if (t < FORM_SCATTER_TICKS + FORM_HOLD_TICKS + FORM_REUNITE_TICKS) p->form_state = FORM_REUNITE;
  else {
    p->form_state = FORM_NONE;
    p->form_t = 0;
    p->form_cd = FORM_COOLDOWN_TICKS;
    p->gauge = 0;
    p->invuln = 45;
    for (int i = 0; i < NUM_COMPONENTS; i++)
      if (p->comp[i]) sim_fx(g, FX_DOCK, p->pos.x + kSlots[i].x, p->pos.y + kSlots[i].y, false);
    sim_sfx(g, SFX_FORMATION_END);
    return;
  }
  /* the gauge doubles as a countdown while the formation is out */
  int total = FORM_SCATTER_TICKS + FORM_HOLD_TICKS + FORM_REUNITE_TICKS;
  p->gauge = GAUGE_MAX - GAUGE_MAX * t / total;
  if (p->form_state != FORM_REUNITE) fire_formation_components(g);
}

/* ---------------- per-tick update ---------------- */

void player_update(Game* g) {
  Player* p = &g->player;
  const Input* in = &g->in;
  if (p->state == PS_DEAD) {
    p->respawn_timer--;
    if (p->respawn_timer <= 0 && p->lives > 0) player_reset(g, false);
    return;
  }
  float dx = (in->right ? 1.0f : 0.0f) - (in->left ? 1.0f : 0.0f);
  float dy = (in->down ? 1.0f : 0.0f) - (in->up ? 1.0f : 0.0f);
  if (dx != 0 && dy != 0) {
    dx *= 0.70710678f;
    dy *= 0.70710678f;
  }
  float sp = player_speed(g);
  p->pos.x += dx * sp;
  p->pos.y += dy * sp;
  if (p->pos.x < PLAYER_MARGIN_X) p->pos.x = PLAYER_MARGIN_X;
  if (p->pos.x > SCREEN_W - PLAYER_MARGIN_X) p->pos.x = SCREEN_W - PLAYER_MARGIN_X;
  if (p->pos.y < PLAYER_TOP) p->pos.y = PLAYER_TOP;
  if (p->pos.y > PLAYER_BOTTOM) p->pos.y = PLAYER_BOTTOM;
  p->bank = dx < -0.1f ? -1 : (dx > 0.1f ? 1 : 0);

  if (p->invuln > 0) p->invuln--;
  if (p->hit_flash > 0) p->hit_flash--;
  for (int i = 0; i < NUM_COMPONENTS; i++)
    if (p->comp[i] && p->dock_t[i] < DOCK_TICKS) p->dock_t[i]++;

  p->fire_duty += ((in->fire ? 1.0f : 0.0f) - p->fire_duty) / 180.0f;
  if (p->shot_cd > 0) p->shot_cd--;
  if (in->fire && p->shot_cd <= 0) {
    fire_volley(g);
    p->shot_cd = player_weapon_interval(g);
  }

  if (in->formation && !g->prev_in.formation && player_can_start_formation(g)) player_start_formation(g);
  formation_update(g);
}
