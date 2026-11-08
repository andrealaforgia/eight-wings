#include "game/game.h"

#include <string.h>

#include "game/sim.h"

/* ================= helpers ================= */

void sim_sfx(Game* g, Sfx s) {
  if (!g->events) return;
  game_event_t event = {GAME_EVENT_SFX + (int)s, NULL, 0};
  publish(g->events, &event);
}

void sim_music(Game* g, Music m) {
  if (m == g->music_current) return;
  g->music_current = m;
  if (!g->events) return;
  game_event_t event = {GAME_EVENT_MUSIC + (int)m, NULL, 0};
  publish(g->events, &event);
}

void sim_add_score(Game* g, long points) {
  Player* p = &g->player;
  p->score += points;
  if (p->score > g->hiscore) g->hiscore = p->score;
  if (p->score >= p->next_life_at) {
    p->lives++;
    p->next_life_at += EXTRA_LIFE_EVERY;
    sim_sfx(g, SFX_ONE_UP);
  }
}

typedef struct {
  int frames, tpf;
} FxInfo;
static const FxInfo kFxInfo[FX_COUNT] = {
    [FX_EXPL_S] = {6, 4}, [FX_EXPL_L] = {8, 5}, [FX_EXPL_G] = {6, 5}, [FX_SPARK] = {4, 3},
    [FX_MUZZLE] = {3, 2}, [FX_DEATH] = {8, 7},  [FX_DOCK] = {4, 5},   [FX_SCORE] = {1, 50},
};
int fx_frame_count(FxType t) { return kFxInfo[t].frames; }
int fx_ticks_per_frame(FxType t) { return kFxInfo[t].tpf; }

Fx* sim_fx(Game* g, FxType type, float x, float y, bool world) {
  Fx* f = POOL_NEW(Fx, g->fx);
  if (!f) return NULL;
  f->type = type;
  f->pos.x = x;
  f->pos.y = world ? sim_ground_wy(g, y) : y;
  f->world = world;
  return f;
}

void sim_fx_score(Game* g, float x, float y, int value) {
  Fx* f = sim_fx(g, FX_SCORE, x, y, false);
  if (f) f->value = value;
}

void sim_charge_gauge(Game* g, int amount) {
  Player* p = &g->player;
  if (p->form_state != FORM_NONE) return;
  p->gauge += amount;
  if (p->gauge > GAUGE_MAX) p->gauge = GAUGE_MAX;
}

float sim_ground_sy(const Game* g, float world_y) { return SCREEN_H - world_y + g->scroll; }
float sim_ground_wy(const Game* g, float screen_y) { return SCREEN_H - screen_y + g->scroll; }

EBullet* sim_ebullet(Game* g, EBulletKind kind, float x, float y, float vx, float vy) {
  EBullet* b = POOL_NEW(EBullet, g->ebullets);
  if (!b) return NULL;
  b->kind = kind;
  b->pos = point(x, y);
  sim_set_velocity(&b->vel, vx, vy);
  return b;
}

float sim_bullet_speed(const Game* g, float base) {
  float k = 0.82f + 0.08f * (float)g->difficulty;
  if (k > 1.7f) k = 1.7f;
  return base * k;
}

int sim_fire_interval(const Game* g, int base) {
  float k = 0.75f + 0.14f * (float)g->difficulty;
  if (k > 2.0f) k = 2.0f;
  int v = (int)((float)base / k);
  return v < 12 ? 12 : v;
}

void sim_aimed_shot(Game* g, float x, float y, float speed, float angle_offset) {
  float dx = 0, dy = 1;
  if (g->player.state == PS_ALIVE) {
    dx = g->player.pos.x - x;
    dy = g->player.pos.y - y;
  }
  float a = atan2f(dx, dy) + angle_offset; /* 0 = straight down */
  float s = sim_bullet_speed(g, speed);
  sim_ebullet(g, EB_ORB, x, y, sinf(a) * s, cosf(a) * s);
}

Pickup* sim_pickup(Game* g, int number, float x, float y) {
  Pickup* p = POOL_NEW(Pickup, g->pickups);
  if (!p) return NULL;
  p->number = number;
  p->pos.x = x;
  p->pos.y = y;
  return p;
}

int sim_next_missing_component(const Game* g) {
  for (int n = 1; n <= NUM_COMPONENTS; n++) {
    if (g->player.comp[n - 1]) continue;
    bool pending = false;
    POOL_EACH(const Pickup, k, g->pickups)
      if (k->number == n) pending = true;
    if (!pending) return n;
  }
  return 0;
}

void sim_kill_all_enemy_bullets_to_fx(Game* g) {
  POOL_EACH(EBullet, b, g->ebullets) {
    sim_fx(g, FX_SPARK, b->pos.x, b->pos.y, false);
    POOL_FREE(g->ebullets, b);
  }
}

bool game_pressed_start(const Game* g) { return g->in.start && !g->prev_in.start; }
static bool pressed(bool now, bool before) { return now && !before; }

/* ================= lifecycle ================= */

void game_init(Game* g, uint32_t seed) {
  long hi = g->hiscore;
  Settings st = g->settings;
  bool had = st.music_vol || st.sfx_vol || st.muted;
  bool dbg = g->debug_overlay, god = g->god_mode;
  event_system_ptr events = g->events;
  object_pool_t* pools[] = {&g->pbullets, &g->ebullets, &g->enemies, &g->ground, &g->pickups, &g->fx};
  const size_t sizes[] = {sizeof(PBullet), sizeof(EBullet), sizeof(Enemy), sizeof(Ground), sizeof(Pickup), sizeof(Fx)};
  const size_t caps[] = {MAX_PBULLETS, MAX_EBULLETS, MAX_ENEMIES, MAX_GROUND, MAX_PICKUPS, MAX_FX};
  object_pool_t kept[6];
  for (int i = 0; i < 6; i++) kept[i] = *pools[i];
  memset(g, 0, sizeof *g);
  for (int i = 0; i < 6; i++) {
    *pools[i] = kept[i].objects ? kept[i] : create_object_pool(sizes[i], caps[i]);
    pool_reset(pools[i]);
  }
  g->events = events;
  g->hiscore = hi;
  g->settings = had ? st : (Settings){8, 8, false};
  g->debug_overlay = dbg;
  g->god_mode = god;
  g->seed = seed ? seed : 1985;
  rng_seed(&g->rng, g->seed);
  g->mode = MODE_TITLE;
  sim_music(g, MUS_TITLE);
  g->player.state = PS_DEAD;
  g->stage_length = level_length(0);
  level_build(g, 0);
  g->scroll_speed = 0.6f;
}

static void begin_stage(Game* g, int stage) {
  g->stage = stage;
  g->difficulty = stage + NUM_STAGES * g->loop;
  g->scroll = 0;
  g->scroll_speed = 0.75f + 0.1f * (float)stage + 0.04f * (float)g->loop;
  g->stage_length = level_length(stage);
  g->phase = PH_INTRO;
  g->phase_tick = 0;
  pool_reset(&g->pbullets);
  pool_reset(&g->ebullets);
  pool_reset(&g->enemies);
  pool_reset(&g->ground);
  pool_reset(&g->pickups);
  pool_reset(&g->fx);
  memset(&g->boss, 0, sizeof g->boss);
  level_build(g, stage);
  sim_music(g, (Music)(MUS_STAGE1 + stage));
}

void game_new_run(Game* g, int start_stage) {
  rng_t keep = g->rng;
  (void)keep;
  g->loop = 0;
  g->kills = 0;
  g->stats_docks = g->stats_formations = g->stats_bosses = 0;
  g->paused = false;
  g->menu_cursor = 0;
  memset(&g->player, 0, sizeof g->player);
  g->player.lives = PLAYER_START_LIVES;
  g->player.next_life_at = EXTRA_LIFE_FIRST;
  g->mode = MODE_PLAYING;
  g->mode_tick = 0;
  begin_stage(g, start_stage < 0 ? 0 : start_stage % NUM_STAGES);
  player_reset(g, true);
  rng_seed(&g->rng, g->seed);
}

/* ================= per-tick simulation ================= */

static void update_pbullets(Game* g) {
  POOL_EACH(PBullet, b, g->pbullets) {
    sim_move(&b->pos, &b->vel);
    if (b->pos.y < -24 || b->pos.y > SCREEN_H + 24 || b->pos.x < -24 || b->pos.x > SCREEN_W + 24) POOL_FREE(g->pbullets, b);
  }
}

static void update_ebullets(Game* g) {
  POOL_EACH(EBullet, b, g->ebullets) {
    b->age++;
    if (b->kind == EB_MISSILE) {
      float vy = sim_vy(&b->vel) + b->accel;
      if (vy > 3.2f) vy = 3.2f;
      sim_set_velocity(&b->vel, sim_vx(&b->vel), vy);
    }
    sim_move(&b->pos, &b->vel);
    if (b->pos.y < -16 || b->pos.y > SCREEN_H + 16 || b->pos.x < -16 || b->pos.x > SCREEN_W + 16) POOL_FREE(g->ebullets, b);
  }
}

static void update_pickups(Game* g) {
  POOL_EACH(Pickup, p, g->pickups) {
    p->age++;
    p->pos.y += g->scroll_speed * 0.9f + 0.15f;
    p->pos.x += sinf((float)p->age * 0.05f) * 0.35f;
    if (p->pos.x < 10) p->pos.x = 10;
    if (p->pos.x > SCREEN_W - 10) p->pos.x = SCREEN_W - 10;
    if (p->pos.y > SCREEN_H + 12) POOL_FREE(g->pickups, p);
  }
}

static void update_fx(Game* g) {
  POOL_EACH(Fx, f, g->fx) {
    f->t++;
    if (f->t >= kFxInfo[f->type].frames * kFxInfo[f->type].tpf) POOL_FREE(g->fx, f);
  }
}

static bool rects_overlap(float ax, float ay, float ahw, float ahh, float bx, float by, float bhw, float bhh) {
  return sim_overlap(ax, ay, ahw, ahh, bx, by, bhw, bhh);
}

static void bullet_half(PBulletKind k, float* hw, float* hh) {
  switch (k) {
    case PB_WIDE: *hw = 3; *hh = 5; break;
    case PB_FORM: *hw = 3; *hh = 5; break;
    case PB_PHOENIX: *hw = 8; *hh = 11; break;
    case PB_HEAVY: *hw = 3; *hh = 6; break;
    default: *hw = 2; *hh = 5; break;
  }
}

static void collide_player_bullets(Game* g) {
  POOL_EACH(PBullet, b, g->pbullets) {
    float bhw, bhh;
    bullet_half(b->kind, &bhw, &bhh);
    bool consumed = false;
    POOL_EACH(Enemy, e, g->enemies) {
      if (consumed) break;
      float hw, hh;
      enemy_half_size(e->type, &hw, &hh);
      if (rects_overlap(b->pos.x, b->pos.y, bhw, bhh, e->pos.x, e->pos.y, hw, hh)) {
        enemy_hit(g, e, b->dmg);
        sim_fx(g, FX_SPARK, b->pos.x, b->pos.y - 2, false);
        if (!b->pierce) consumed = true;
      }
    }
    if (!consumed && b->pos.y > 0) {
      POOL_EACH(Ground, e, g->ground) {
        if (consumed) break;
        if (e->hp <= 0) continue;
        float hw, hh;
        ground_half_size(e->type, &hw, &hh);
        float sy = sim_ground_sy(g, e->pos.y);
        if (rects_overlap(b->pos.x, b->pos.y, bhw, bhh, e->pos.x, sy, hw, hh)) {
          ground_hit(g, e, b->dmg);
          sim_fx(g, FX_SPARK, b->pos.x, b->pos.y - 2, false);
          if (!b->pierce) consumed = true;
        }
      }
    }
    if (!consumed && g->boss.active && boss_hit_test(g, b)) consumed = true;
    if (consumed) POOL_FREE(g->pbullets, b);
  }
}

static void collide_enemy_bullets(Game* g) {
  Player* p = &g->player;
  if (p->state != PS_ALIVE) return;
  POOL_EACH(EBullet, b, g->ebullets) {
    float r = b->kind == EB_MISSILE ? 3.0f : 2.2f;
    /* detached formation components and the phoenix absorb bullets */
    if (p->form_state != FORM_NONE) {
      bool blocked = player_is_phoenix(g) && rects_overlap(b->pos.x, b->pos.y, r, r, p->pos.x, p->pos.y, 14, 14);
      for (int k = 0; k < NUM_COMPONENTS && !blocked; k++) {
        float x, y, hw, hh;
        if (player_component_rect(g, k, &x, &y, &hw, &hh) && rects_overlap(b->pos.x, b->pos.y, r, r, x, y, hw + 1, hh + 1)) blocked = true;
      }
      if (blocked) {
        POOL_FREE(g->ebullets, b);
        sim_fx(g, FX_SPARK, b->pos.x, b->pos.y, false);
        continue;
      }
    }
    if (player_try_hit(g, b->pos.x, b->pos.y, r)) POOL_FREE(g->ebullets, b);
    if (p->state != PS_ALIVE) return;
  }
}

static void collide_enemy_bodies(Game* g) {
  Player* p = &g->player;
  if (p->state != PS_ALIVE) return;
  POOL_EACH(Enemy, e, g->enemies) {
    float hw, hh;
    enemy_half_size(e->type, &hw, &hh);
    /* components flying in formation (and the phoenix) ram enemies */
    if (p->form_state == FORM_HOLD || p->form_state == FORM_SCATTER) {
      for (int k = 0; k < NUM_COMPONENTS; k++) {
        float x, y, chw, chh;
        if (player_component_rect(g, k, &x, &y, &chw, &chh) && rects_overlap(x, y, chw, chh, e->pos.x, e->pos.y, hw, hh) && g->tick % 5 == 0)
          enemy_hit(g, e, 2);
      }
      if (player_is_phoenix(g) && rects_overlap(p->pos.x, p->pos.y, 13, 13, e->pos.x, e->pos.y, hw, hh) && g->tick % 4 == 0) enemy_hit(g, e, 3);
      if (!POOL_ALIVE(g->enemies, e)) continue;
    }
    if (player_hit_rect(g, e->pos.x, e->pos.y, hw * 0.8f, hh * 0.8f)) {
      /* a kamikaze that hurts you also dies, except heavies */
      if (e->type != ET_BOMBER) enemy_hit(g, e, 99);
    }
    if (p->state != PS_ALIVE) return;
  }
}

static void collect_pickups(Game* g) {
  Player* p = &g->player;
  if (p->state != PS_ALIVE) return;
  POOL_EACH(Pickup, k, g->pickups) {
    if (fabsf(k->pos.x - p->pos.x) < 13 && fabsf(k->pos.y - p->pos.y) < 15) {
      POOL_FREE(g->pickups, k);
      if (k->number == 0) {
        sim_add_score(g, 1000);
        sim_fx_score(g, k->pos.x, k->pos.y, 1000);
        sim_charge_gauge(g, 10);
        sim_sfx(g, SFX_MEDAL);
      } else if (!p->comp[k->number - 1]) {
        player_dock(g, k->number, (Vec2){k->pos.x - p->pos.x, k->pos.y - p->pos.y});
      } else {
        sim_add_score(g, 1000);
        sim_fx_score(g, k->pos.x, k->pos.y, 1000);
        sim_sfx(g, SFX_MEDAL);
      }
    }
  }
}

static void stage_flow(Game* g) {
  g->phase_tick++;
  switch (g->phase) {
    case PH_INTRO:
      if (g->phase_tick >= STAGE_INTRO_TICKS) { g->phase = PH_PLAY; g->phase_tick = 0; }
      break;
    case PH_PLAY:
      if (g->scroll >= g->stage_length) {
        g->phase = PH_BOSS;
        g->phase_tick = 0;
        sim_music(g, MUS_BOSS);
        sim_sfx(g, SFX_BOSS_WARNING);
        boss_start(g);
      }
      break;
    case PH_BOSS:
      if (!g->boss.active) {
        g->phase = PH_CLEAR;
        g->phase_tick = 0;
        sim_music(g, MUS_CLEAR);
        sim_sfx(g, SFX_STAGE_CLEAR);
        g->stats_bosses++;
        sim_add_score(g, 5000 * (g->stage + 1));
        sim_kill_all_enemy_bullets_to_fx(g);
      }
      break;
    case PH_CLEAR:
      if (g->phase_tick >= STAGE_CLEAR_TICKS) {
        int next = g->stage + 1;
        if (next >= NUM_STAGES) { next = 0; g->loop++; }
        begin_stage(g, next);
      }
      break;
  }
}

static void end_run(Game* g) {
  g->mode = MODE_GAMEOVER;
  g->mode_tick = 0;
  sim_music(g, MUS_GAMEOVER);
}

static void simulate(Game* g, const Input* in) {
  (void)in;
  stage_flow(g);
  level_update(g);
  player_update(g);
  enemy_update_all(g);
  ground_update_all(g);
  boss_update(g);
  update_pbullets(g);
  update_ebullets(g);
  collide_player_bullets(g);
  collide_enemy_bullets(g);
  collide_enemy_bodies(g);
  update_pickups(g);
  collect_pickups(g);
  update_fx(g);
  if (g->screen_flash > 0) g->screen_flash -= 0.04f;
  if (g->shake > 0) g->shake--;
  if (g->player.state == PS_DEAD && g->player.lives <= 0 && g->player.respawn_timer <= 0) {
    if (g->mode == MODE_DEMO) {
      g->mode = MODE_TITLE;
      g->mode_tick = 0;
      sim_music(g, MUS_TITLE);
    } else {
      end_run(g);
    }
  }
}

static bool any_input(const Input* in) {
  return in->up || in->down || in->left || in->right || in->fire || in->formation || in->start || in->pause;
}

static void pause_menu(Game* g) {
  Input* in = &g->in;
  Input* pv = &g->prev_in;
  Settings* s = &g->settings;
  if (pressed(in->pause, pv->pause)) { g->paused = false; sim_sfx(g, SFX_PAUSE); return; }
  if (pressed(in->up, pv->up)) { g->menu_cursor = (g->menu_cursor + 4) % 5; sim_sfx(g, SFX_MENU_MOVE); }
  if (pressed(in->down, pv->down)) { g->menu_cursor = (g->menu_cursor + 1) % 5; sim_sfx(g, SFX_MENU_MOVE); }
  int adj = (pressed(in->right, pv->right) ? 1 : 0) - (pressed(in->left, pv->left) ? 1 : 0);
  bool enter = pressed(in->start, pv->start);
  switch (g->menu_cursor) {
    case 0:
      if (enter) { g->paused = false; sim_sfx(g, SFX_MENU_SELECT); }
      break;
    case 1:
      if (adj) { s->music_vol += adj; if (s->music_vol < 0) s->music_vol = 0; if (s->music_vol > 10) s->music_vol = 10; sim_sfx(g, SFX_MENU_MOVE); }
      break;
    case 2:
      if (adj) { s->sfx_vol += adj; if (s->sfx_vol < 0) s->sfx_vol = 0; if (s->sfx_vol > 10) s->sfx_vol = 10; sim_sfx(g, SFX_MENU_MOVE); }
      break;
    case 3:
      if (enter || adj) { s->muted = !s->muted; sim_sfx(g, SFX_MENU_SELECT); }
      break;
    case 4:
      if (enter) { g->paused = false; g->mode = MODE_TITLE; g->mode_tick = 0; sim_music(g, MUS_TITLE); sim_sfx(g, SFX_MENU_SELECT); }
      break;
  }
}

void game_tick(Game* g, const Input* in_raw) {
  g->prev_in = g->in;
  g->in = *in_raw;
  g->tick++;
  g->mode_tick++;

  switch (g->mode) {
    case MODE_TITLE:
    case MODE_CONTROLS:
      sim_music(g, MUS_TITLE);
      g->scroll += 0.6f;
      if (g->scroll > 1500) g->scroll = 0;
      if (game_pressed_start(g)) {
        sim_sfx(g, SFX_MENU_SELECT);
        game_new_run(g, 0);
        return;
      }
      if (g->mode == MODE_TITLE && g->mode_tick > TITLE_TO_CONTROLS_TICKS) { g->mode = MODE_CONTROLS; g->mode_tick = 0; }
      else if (g->mode == MODE_CONTROLS && g->mode_tick > CONTROLS_TO_DEMO_TICKS) {
        uint32_t keep_seed = g->seed;
        long hi = g->hiscore;
        game_new_run(g, 0);
        g->mode = MODE_DEMO;
        g->mode_tick = 0;
        g->hiscore = hi;
        g->seed = keep_seed;
      }
      /* from the title, the pause key toggles the controls page by hand */
      if (pressed(g->in.pause, g->prev_in.pause)) { g->mode = (g->mode == MODE_TITLE) ? MODE_CONTROLS : MODE_TITLE; g->mode_tick = 0; sim_sfx(g, SFX_MENU_MOVE); }
      return;
    case MODE_GAMEOVER:
      if (g->mode_tick > 90 && game_pressed_start(g)) { sim_sfx(g, SFX_MENU_SELECT); game_new_run(g, 0); return; }
      if (g->mode_tick > GAMEOVER_TICKS) { g->mode = MODE_TITLE; g->mode_tick = 0; sim_music(g, MUS_TITLE); }
      update_fx(g);
      return;
    case MODE_DEMO: {
      if (any_input(in_raw) || g->mode_tick > DEMO_TICKS) {
        g->mode = MODE_TITLE;
        g->mode_tick = 0;
        sim_music(g, MUS_TITLE);
        return;
      }
      Input ai;
      demo_ai(g, &ai);
      g->in = ai;
      simulate(g, &ai);
      return;
    }
    case MODE_PLAYING:
      if (g->paused) { pause_menu(g); return; }
      if (pressed(g->in.pause, g->prev_in.pause)) { g->paused = true; g->menu_cursor = 0; sim_sfx(g, SFX_PAUSE); return; }
      simulate(g, &g->in);
      return;
  }
}

void game_free(Game* g) {
  pool_destroy(&g->pbullets);
  pool_destroy(&g->ebullets);
  pool_destroy(&g->enemies);
  pool_destroy(&g->ground);
  pool_destroy(&g->pickups);
  pool_destroy(&g->fx);
}
