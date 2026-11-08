#include "platform/scene.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "game/sim.h"

#define S SPRITE_DEFAULT

static SpriteStyle style(float alpha) {
  SpriteStyle s = SPRITE_DEFAULT;
  s.alpha = alpha;
  return s;
}
static SpriteStyle tint(float rr, float gg, float bb) {
  SpriteStyle s = SPRITE_DEFAULT;
  s.r = rr; s.g = gg; s.b = bb;
  return s;
}
static SpriteStyle additive(float alpha) {
  SpriteStyle s = SPRITE_DEFAULT;
  s.additive = true;
  s.alpha = alpha;
  return s;
}

/* ---------- terrain ---------- */

static void draw_terrain(Renderer* r, const Game* g) {
  int theme = level_theme(g->stage);
  float scroll = floorf(g->scroll);
  int r0 = (int)floorf(scroll / TILE) - 1;
  int r1 = (int)floorf((scroll + SCREEN_H) / TILE) + 1;
  for (int row = r0; row <= r1; row++) {
    float y = SCREEN_H - (float)(row + 1) * TILE + scroll;
    if (y > SCREEN_H || y < -TILE) continue;
    for (int col = 0; col < MAP_COLS; col++) {
      uint16_t t = level_tile(g, row, col);
      int local = level_tile_sprite_local(t);
      renderer_sprite(r, SPR_TILE(theme, local), 0, (float)col * TILE + 8, y + 8, &S);
    }
  }
}

static void draw_clouds(Renderer* r, const Game* g) {
  /* a sparse parallax layer scrolling faster than the ground gives the impression of altitude */
  SpriteStyle st = style(0.55f);
  float sc = g->scroll * 1.6f;
  for (int i = 0; i < 7; i++) {
    float period = 520.0f;
    float y = fmodf(sc + (float)i * 131.0f, period) - 40.0f;
    if (y > SCREEN_H + 20) continue;
    float x = fmodf((float)(i * 97 + 40) + (float)(g->stage * 53), 300.0f) - 20.0f;
    renderer_sprite(r, SPR_cloud, i % 3, x, y, &st);
  }
}

/* ---------- entities ---------- */

static void shadow(Renderer* r, SpriteId id, int frame, float x, float y, float rot) {
  SpriteStyle st = tint(0, 0, 0);
  st.alpha = 0.32f;
  st.rot = rot;
  renderer_sprite(r, id, frame, x + 6, y + 9, &st);
}

/* snap headings to 22.5 degree steps so rotated pixel art stays crisp and consistent between frames */
static float quantise(float a) {
  const float step = 0.39269908f;
  return floorf(a / step + 0.5f) * step;
}

static void draw_ground(Renderer* r, const Game* g) {
  POOL_EACH(const Ground, e, g->ground) {
    float sy = sim_ground_sy(g, e->pos.y);
    if (sy < -24 || sy > SCREEN_H + 24) continue;
    float x = e->pos.x;
    SpriteStyle st = S;
    if (e->hp <= 0) {
      if (e->type == GT_RAPTOR || e->type == GT_REX) {
        /* the skeleton lies in the pose the creature died in */
        SpriteStyle bs = S;
        bs.rot = -quantise(e->p0);
        renderer_sprite(r, e->type == GT_REX ? SPR_g_bones_l : SPR_g_bones_s, 0, x, sy, &bs);
      } else {
        renderer_sprite(r, e->type == GT_FORT ? SPR_g_rubble_l : SPR_g_rubble_s, 0, x, sy, &S);
      }
      continue;
    }
    switch (e->type) {
      case GT_TURRET: renderer_sprite(r, SPR_g_turret, e->aim, x, sy, &st); break;
      case GT_BUNKER: renderer_sprite(r, SPR_g_bunker, e->hp <= 2 ? 2 : e->state, x, sy, &st); break;
      case GT_TANK: renderer_sprite(r, SPR_g_tank, (e->age / 8) % 2, x, sy, &st); break;
      case GT_SILO: renderer_sprite(r, SPR_g_silo, e->state, x, sy, &st); break;
      case GT_FORT: renderer_sprite(r, SPR_g_fort, e->state > 0 ? 1 : 0, x, sy, &st); break;
      case GT_RADAR: renderer_sprite(r, SPR_g_radar, (e->age / 6) % 4, x, sy, &st); break;
      case GT_RAPTOR: { st.rot = -quantise(e->p0); renderer_sprite(r, SPR_g_raptor, (e->age / 5) % 4, x, sy, &st); break; }
      case GT_REX: { st.rot = -quantise(e->p0); renderer_sprite(r, SPR_g_rex, (e->age / 9) % 4, x, sy, &st); break; }
      default: break;
    }
    if (e->flash > 0) {
      SpriteStyle fl = additive(0.7f);
      SpriteId id = e->type == GT_TURRET ? SPR_g_turret : e->type == GT_BUNKER ? SPR_g_bunker : e->type == GT_TANK ? SPR_g_tank : e->type == GT_SILO ? SPR_g_silo : e->type == GT_FORT ? SPR_g_fort : e->type == GT_RAPTOR ? SPR_g_raptor : e->type == GT_REX ? SPR_g_rex : SPR_g_radar;
      int fr = e->type == GT_TURRET ? e->aim : e->type == GT_BUNKER ? (e->hp <= 2 ? 2 : e->state) : e->type == GT_RAPTOR ? (e->age / 5) % 4 : e->type == GT_REX ? (e->age / 9) % 4 : 0;
      if (e->type == GT_RAPTOR || e->type == GT_REX) fl.rot = -quantise(e->p0);
      renderer_sprite(r, id, fr, x, sy, &fl);
    }
  }
}

static void draw_pickups(Renderer* r, const Game* g, long frame) {
  POOL_EACH(const Pickup, p, g->pickups) {
    int f = (int)(frame / 8) % 2;
    if (p->number == 0) {
      renderer_sprite(r, SPR_medal, f, p->pos.x, p->pos.y, &S);
    } else {
      SpriteStyle glow = additive(0.25f + 0.2f * sinf((float)frame * 0.2f));
      glow.scale = 1.5f;
      SpriteId id = (SpriteId)(SPR_pod_1 + p->number - 1);
      renderer_sprite(r, id, f, p->pos.x, p->pos.y, &glow);
      renderer_sprite(r, id, f, p->pos.x, p->pos.y, &S);
    }
  }
}

static SpriteId enemy_sprite(EnemyType t, int* nframes) {
  switch (t) {
    case ET_DRONE: *nframes = 4; return SPR_e_drone;
    case ET_SWOOP: *nframes = 4; return SPR_e_swoop;
    case ET_DIVER: *nframes = 4; return SPR_e_diver;
    case ET_SPINNER: *nframes = 4; return SPR_e_spinner;
    case ET_WING: *nframes = 2; return SPR_e_wing;
    case ET_BOMBER: *nframes = 2; return SPR_e_bomber;
    default: *nframes = 4; return SPR_e_saucer;
  }
}

static void draw_enemies(Renderer* r, const Game* g) {
  POOL_EACH(const Enemy, e, g->enemies) {
    int n;
    SpriteId id = enemy_sprite(e->type, &n);
    int tpf = e->type == ET_DIVER ? 3 : (e->type == ET_BOMBER ? 8 : 5);
    int fr = (e->age / tpf) % n;
    float rot = 0;
    if (e->type == ET_SWOOP) rot = -e->p0;               /* sprite faces down; p0 = heading from down toward +x */
    if (e->type == ET_DIVER && e->state == 2) rot = -atan2f(sim_vx(&e->vel), sim_vy(&e->vel));
    SpriteStyle st = S;
    st.rot = rot;
    shadow(r, id, fr, e->pos.x, e->pos.y, rot);
    if (e->type == ET_DIVER && e->state == 1 && (e->timer / 3) % 2 == 0) st = tint(1.0f, 0.7f, 0.7f);
    renderer_sprite(r, id, fr, e->pos.x, e->pos.y, &st);
    if (e->flash > 0) {
      SpriteStyle fl = additive(0.75f);
      fl.rot = rot;
      renderer_sprite(r, id, fr, e->pos.x, e->pos.y, &fl);
    }
  }
}

static void draw_boss(Renderer* r, const Game* g, long frame) {
  const Boss* b = &g->boss;
  if (!b->active) return;
  SpriteId body = (SpriteId)(SPR_boss1_body + b->id * 3);
  SpriteId core = (SpriteId)(SPR_boss1_core + b->id * 3);
  SpriteId gun = (SpriteId)(SPR_boss1_gun + b->id * 3);
  float shk = (b->dying > 0) ? (float)((frame % 3) - 1) * 1.0f : 0.0f;
  float bx = b->pos.x + shk, by = b->pos.y;
  shadow(r, body, 0, bx, by, 0);
  renderer_sprite(r, body, 0, bx, by, &S);
  if (b->flash_body > 0) { SpriteStyle fl = additive(0.25f); renderer_sprite(r, body, 0, bx, by, &fl); }
  for (int i = 0; i < 2; i++) {
    float gx, gy;
    boss_gun_pos(b, i, &gx, &gy);
    gx += shk;
    if (b->gun_hp[i] > 0) {
      int fi = b->t % 75;
      int fr = fi < 60 ? 0 : (fi < 68 ? 1 : (fi < 72 ? 2 : 3));
      renderer_sprite(r, gun, fr, gx, gy, &S);
      if (b->flash_gun[i] > 0) { SpriteStyle fl = additive(0.8f); renderer_sprite(r, gun, fr, gx, gy, &fl); }
    } else {
      renderer_sprite(r, SPR_g_rubble_s, 0, gx, gy, &S);
    }
  }
  float cx, cy;
  boss_core_pos(b, &cx, &cy);
  cx += shk;
  if (b->phase == 0) {
    SpriteStyle sh = tint(0.45f, 0.55f, 0.9f);
    renderer_sprite(r, core, 0, cx, cy, &sh); /* shielded: cold blue */
  } else {
    int fr = (b->flash_core > 0 || (frame / (b->phase == 2 ? 4 : 8)) % 2 == 0) ? 1 : 0;
    SpriteStyle glow = additive(0.3f + 0.2f * sinf((float)frame * 0.25f));
    glow.scale = 1.4f;
    renderer_sprite(r, core, fr, cx, cy, &glow);
    renderer_sprite(r, core, fr, cx, cy, &S);
  }
  /* beams: telegraph, then the real thing */
  for (int i = 0; i < MAX_BEAMS; i++) {
    const Beam* m = &b->beams[i];
    float y0 = b->pos.y + 30;
    if (m->warn > 0) {
      if ((m->warn / 3) % 2 == 0) renderer_rect(r, m->x - 1, y0, 2, SCREEN_H - y0, 1.0f, 0.5f, 0.8f, 0.7f);
    } else if (m->on > 0) {
      for (float y = y0; y < SCREEN_H + 8; y += 16) renderer_sprite(r, SPR_beam, (int)(frame / 3) % 2, m->x, y + 8, &S);
    }
  }
  /* energy bar */
  float total = (float)(b->gun_max * 2 + b->core_max), cur = (float)(b->gun_hp[0] + b->gun_hp[1] + b->core_hp);
  float w = 160.0f * cur / total;
  renderer_rect(r, 48 - 1, 17, 162, 5, 0.04f, 0.04f, 0.08f, 1);
  renderer_rect(r, 48, 18, w, 3, b->phase == 2 ? 1.0f : 0.9f, b->phase == 2 ? 0.35f : 0.65f, 0.15f, 1);
}

static void draw_player(Renderer* r, const Game* g, long frame) {
  const Player* p = &g->player;
  if (p->state != PS_ALIVE) return;
  bool blink = p->invuln > 0 && p->form_state == FORM_NONE && (frame / 3) % 2 == 0 && !g->god_mode;
  float x = p->pos.x, y = p->pos.y;
  bool phoenix = player_is_phoenix(g);
  SpriteStyle st = S;
  if (p->hit_flash > 0 && (p->hit_flash / 2) % 2 == 0) st = tint(1.5f, 1.2f, 1.2f);
  if (blink) return;

  /* afterimages of detached components make the formation read as motion */
  if (p->form_state != FORM_NONE) {
    for (int k = 1; k <= 3; k++) {
      int t = p->form_t - k * 3;
      if (t < 0) continue;
      int rank = 0, count = player_component_count(g);
      for (int i = 0; i < NUM_COMPONENTS; i++) {
        if (!p->comp[i]) continue;
        int stt = t < FORM_SCATTER_TICKS ? FORM_SCATTER : (t < FORM_SCATTER_TICKS + FORM_HOLD_TICKS ? FORM_HOLD : FORM_REUNITE);
        Vec2 o = formation_offset(i, rank++, count, stt, t);
        SpriteStyle gh = additive(0.28f / (float)k);
        renderer_sprite(r, (SpriteId)(SPR_ship_c1 + i), (int)(frame / 4) % 2, x + o.x, y + o.y, &gh);
      }
    }
  }
  /* shadow of the whole craft */
  shadow(r, SPR_ship_core, 0, x, y, 0);
  for (int i = 0; i < NUM_COMPONENTS; i++) {
    if (!p->comp[i]) continue;
    Vec2 o = player_comp_offset(g, i);
    renderer_sprite(r, (SpriteId)(SPR_ship_c1 + i), (int)(frame / 4) % 2, x + o.x, y + o.y, &st);
    if (p->form_state != FORM_NONE) {
      SpriteStyle gl = additive(0.5f + 0.3f * sinf((float)frame * 0.5f + (float)i));
      renderer_sprite(r, (SpriteId)(SPR_ship_c1 + i), (int)(frame / 4) % 2, x + o.x, y + o.y, &gl);
    } else if (p->dock_t[i] < DOCK_TICKS) {
      SpriteStyle gl = additive(0.9f - (float)p->dock_t[i] / DOCK_TICKS);
      renderer_sprite(r, SPR_dock_flash, (int)(frame / 3) % 4, x + o.x, y + o.y, &gl);
    }
  }
  if (phoenix) {
    SpriteStyle aura = additive(0.35f);
    aura.scale = 1.5f;
    renderer_sprite(r, SPR_phoenix, (int)(frame / 4) % 4, x, y, &aura);
    renderer_sprite(r, SPR_phoenix, (int)(frame / 4) % 4, x, y, &st);
  } else {
    SpriteId core = p->bank < 0 ? SPR_ship_core_bank_l : (p->bank > 0 ? SPR_ship_core_bank_r : SPR_ship_core);
    renderer_sprite(r, core, (int)(frame / 3) % 2, x, y, &st);
  }
  if (p->form_state != FORM_NONE && !phoenix) {
    SpriteStyle sh = additive(0.25f + 0.1f * sinf((float)frame * 0.3f));
    sh.r = 0.5f; sh.g = 0.8f;
    renderer_sprite(r, SPR_ship_core, (int)(frame / 3) % 2, x, y, &sh);
  }
}

static void draw_bullets(Renderer* r, const Game* g, long frame) {
  POOL_EACH(const PBullet, b, g->pbullets) {
    switch (b->kind) {
      case PB_NORMAL: renderer_sprite(r, SPR_shot_p, 0, b->pos.x, b->pos.y, &S); break;
      case PB_HEAVY: { SpriteStyle st = S; st.scale = 1.0f; renderer_sprite(r, SPR_shot_w, 0, b->pos.x, b->pos.y, &st); break; }
      case PB_WIDE: renderer_sprite(r, SPR_shot_w, 0, b->pos.x, b->pos.y, &S); break;
      case PB_FORM: {
        SpriteStyle st = S;
        st.rot = atan2f(sim_vx(&b->vel), -sim_vy(&b->vel));
        renderer_sprite(r, SPR_shot_f, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &st);
        break;
      }
      case PB_BOLT: { SpriteStyle st = S; st.g = 0.85f; st.b = 0.5f; renderer_sprite(r, SPR_shot_w, 0, b->pos.x, b->pos.y, &st); break; }
      case PB_LANCE: {
        SpriteStyle st = S;
        st.rot = atan2f(sim_vx(&b->vel), -sim_vy(&b->vel));
        st.scale = 1.25f;
        renderer_sprite(r, SPR_shot_f, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &st);
        SpriteStyle gl = additive(0.5f);
        gl.rot = st.rot; gl.scale = 1.5f;
        renderer_sprite(r, SPR_shot_f, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &gl);
        break;
      }
      case PB_PLASMA: {
        SpriteStyle st = S;
        st.scale = 0.7f; st.r = 0.5f; st.g = 1.0f; st.b = 0.7f;
        renderer_sprite(r, SPR_shot_ph, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &st);
        SpriteStyle gl = additive(0.45f + 0.15f * sinf((float)frame * 0.5f));
        gl.scale = 0.95f; gl.r = 0.4f; gl.b = 0.6f;
        renderer_sprite(r, SPR_shot_ph, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &gl);
        break;
      }
      case PB_PHOENIX: renderer_sprite(r, SPR_shot_ph, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &S); break;
    }
  }
  POOL_EACH(const EBullet, b, g->ebullets) {
    if (b->kind == EB_MISSILE) renderer_sprite(r, SPR_bullet_m, (int)(frame / 3) % 2, b->pos.x, b->pos.y, &S);
    else renderer_sprite(r, SPR_bullet_e, (b->age / 4) % 4, b->pos.x, b->pos.y, &S);
  }
}

static void draw_fx(Renderer* r, const Game* g) {
  POOL_EACH(const Fx, f, g->fx) {
    float x = f->pos.x;
    float y = f->world ? sim_ground_sy(g, f->pos.y) : f->pos.y;
    int fr = f->t / fx_ticks_per_frame(f->type);
    if (fr >= fx_frame_count(f->type)) fr = fx_frame_count(f->type) - 1;
    switch (f->type) {
      case FX_EXPL_S: renderer_sprite(r, SPR_expl_s, fr, x, y, &S); break;
      case FX_EXPL_L: renderer_sprite(r, SPR_expl_l, fr, x, y, &S); break;
      case FX_EXPL_G: renderer_sprite(r, SPR_expl_g, fr, x, y, &S); break;
      case FX_SPARK: renderer_sprite(r, SPR_spark, fr, x, y, &S); break;
      case FX_MUZZLE: renderer_sprite(r, SPR_muzzle, fr, x, y, &S); break;
      case FX_DEATH: renderer_sprite(r, SPR_ship_death, fr, x, y, &S); break;
      case FX_DOCK: { SpriteStyle a = additive(1.0f); renderer_sprite(r, SPR_dock_flash, fr, x, y, &a); break; }
      case FX_SCORE: {
        char buf[16];
        snprintf(buf, sizeof buf, "%d", f->value);
        float a = f->t > 36 ? 1.0f - (float)(f->t - 36) / 14.0f : 1.0f;
        renderer_text(r, buf, x - (float)renderer_text_width(buf) / 2, y - (float)f->t * 0.25f, 1.0f, 0.95f, 0.5f, a);
        break;
      }
      default: break;
    }
  }
}

/* ---------- HUD ---------- */

static void text_center(Renderer* r, const char* s, float y, float cr, float cg, float cb, float a) {
  renderer_text(r, s, (SCREEN_W - (float)renderer_text_width(s)) / 2, y, cr, cg, cb, a);
}

static void draw_hud(Renderer* r, const Game* g, long frame) {
  const Player* p = &g->player;
  char buf[64];
  renderer_rect(r, 0, 0, SCREEN_W, 11, 0.02f, 0.02f, 0.06f, 0.55f);
  snprintf(buf, sizeof buf, "1UP %06ld", p->score);
  renderer_text(r, buf, 4, 2, 1.0f, 1.0f, 1.0f, 1);
  {
    int t = sim_fire_bonus_tenths(g);
    snprintf(buf, sizeof buf, "X%d.%d", t / 10, t % 10);
    renderer_text(r, buf, 4, 13, t >= 15 ? 0.3f : 1.0f, 1.0f, t >= 15 ? 0.3f : 0.3f, 0.9f);
  }
  snprintf(buf, sizeof buf, "HI %06ld", g->hiscore);
  renderer_text(r, buf, 100, 2, 1.0f, 0.85f, 0.3f, 1);
  snprintf(buf, sizeof buf, "ST%d%s", g->stage + 1, g->loop ? "L" : "");
  renderer_text(r, buf, SCREEN_W - 4 - (float)renderer_text_width(buf), 2, 0.6f, 0.9f, 1.0f, 1);

  /* bottom bar: lives, component slots, formation gauge */
  renderer_rect(r, 0, SCREEN_H - 12, SCREEN_W, 12, 0.02f, 0.02f, 0.06f, 0.55f);
  for (int i = 0; i < p->lives && i < 5; i++) renderer_sprite(r, SPR_ui_life, 0, 8 + (float)i * 10, SCREEN_H - 6, &S);
  for (int i = 0; i < NUM_COMPONENTS; i++) {
    SpriteStyle st = p->comp[i] ? S : style(0.25f);
    renderer_sprite(r, SPR_ui_comp, i, 66 + (float)i * 9, SCREEN_H - 6, &st);
  }
  float gx = 144, gw = 72;
  renderer_rect(r, gx - 1, SCREEN_H - 9, gw + 2, 6, 0.1f, 0.1f, 0.2f, 1);
  bool ready = player_can_start_formation(g);
  float fill = gw * (float)p->gauge / GAUGE_MAX;
  float pulse = ready ? 0.6f + 0.4f * sinf((float)frame * 0.4f) : 1.0f;
  if (p->form_state != FORM_NONE) renderer_rect(r, gx, SCREEN_H - 8, fill, 4, 1.0f, 0.75f, 0.2f, 1);
  else renderer_rect(r, gx, SCREEN_H - 8, fill, 4, 0.2f * pulse, 0.8f * pulse, 1.0f * pulse, 1);
  if (ready) renderer_text(r, "X", gx + gw + 5, SCREEN_H - 10, 1.0f, 0.9f, 0.3f, pulse);
  else if (player_component_count(g) < FORMATION_MIN_COMPONENTS) renderer_text(r, "FORM", gx + gw + 3, SCREEN_H - 10, 0.5f, 0.5f, 0.6f, 1);
}

static void draw_banners(Renderer* r, const Game* g, long frame) {
  char buf[64];
  if (g->phase == PH_INTRO && g->phase_tick < STAGE_INTRO_TICKS && g->mode != MODE_GAMEOVER) {
    float a = g->phase_tick < 20 ? (float)g->phase_tick / 20.0f : (g->phase_tick > STAGE_INTRO_TICKS - 25 ? (float)(STAGE_INTRO_TICKS - g->phase_tick) / 25.0f : 1.0f);
    snprintf(buf, sizeof buf, "STAGE %d", g->stage + 1 + g->loop * NUM_STAGES);
    text_center(r, buf, 84, 1.0f, 1.0f, 1.0f, a);
    static const char* const names[NUM_STAGES] = {"BADLANDS", "ARCHIPELAGO", "THE FORTRESS"};
    text_center(r, names[g->stage], 98, 0.5f, 0.9f, 1.0f, a);
  }
  if (g->phase == PH_BOSS && g->boss.active && g->boss.entering > 0 && (frame / 8) % 2 == 0) text_center(r, "WARNING", 96, 1.0f, 0.2f, 0.25f, 1);
  if (g->phase == PH_CLEAR && g->phase_tick > 40) {
    text_center(r, "STAGE CLEAR", 84, 1.0f, 0.9f, 0.3f, 1);
    snprintf(buf, sizeof buf, "BOSS BONUS %d", 5000 * (g->stage + 1));
    text_center(r, buf, 100, 1.0f, 1.0f, 1.0f, 1);
  }
}

static void draw_pause(Renderer* r, const Game* g) {
  renderer_rect(r, 0, 0, SCREEN_W, SCREEN_H, 0, 0, 0.05f, 0.7f);
  text_center(r, "PAUSED", 56, 1.0f, 1.0f, 1.0f, 1);
  char buf[48];
  const char* items[5];
  items[0] = "RESUME";
  snprintf(buf, sizeof buf, "MUSIC  %d", g->settings.music_vol);
  char b1[48], b2[48];
  snprintf(b1, sizeof b1, "MUSIC  < %2d >", g->settings.music_vol);
  snprintf(b2, sizeof b2, "SFX    < %2d >", g->settings.sfx_vol);
  items[1] = b1;
  items[2] = b2;
  items[3] = g->settings.muted ? "SOUND  OFF" : "SOUND  ON ";
  items[4] = "QUIT TO TITLE";
  for (int i = 0; i < 5; i++) {
    float y = 84 + (float)i * 14;
    bool sel = i == g->menu_cursor;
    renderer_text(r, items[i], 72, y, sel ? 1.0f : 0.6f, sel ? 0.9f : 0.6f, sel ? 0.3f : 0.7f, 1);
    if (sel) renderer_text(r, ">", 58, y, 1.0f, 0.9f, 0.3f, 1);
  }
  text_center(r, "UP/DOWN  LEFT/RIGHT ADJUST", 176, 0.5f, 0.6f, 0.8f, 1);
  text_center(r, "ESC RESUMES", 188, 0.5f, 0.6f, 0.8f, 1);
}

static void draw_title(Renderer* r, const Game* g, long frame) {
  renderer_rect(r, 0, 0, SCREEN_W, SCREEN_H, 0, 0, 0.05f, 0.35f);
  float bob = sinf((float)frame * 0.05f) * 2.0f;
  renderer_sprite(r, SPR_logo_title, 0, 128, 52 + bob, &S);
  SpriteStyle sh = additive(0.12f + 0.08f * sinf((float)frame * 0.1f));
  renderer_sprite(r, SPR_logo_title, 0, 128, 52 + bob, &sh);
  /* the four modules, docking in a loop below the logo */
  int phase = (int)(frame / 28) % 11;
  float cx = 128, cy = 120;
  renderer_sprite(r, SPR_ship_core, (int)(frame / 3) % 2, cx, cy, &S);
  static const float ox[8] = {-12, 12, 0, 0, -22, 22, -10, 10}, oy[8] = {2, 2, -16, 15, 6, 6, -12, -12};
  for (int i = 0; i < 8; i++) {
    float sx = ox[i] * 2.6f, sy = oy[i] * 2.2f + (i == 3 ? 14.0f : 0.0f);
    if (phase > i) renderer_sprite(r, (SpriteId)(SPR_ship_c1 + i), (int)(frame / 4) % 2, cx + ox[i], cy + oy[i], &S);
    else renderer_sprite(r, (SpriteId)(SPR_ship_c1 + i), (int)(frame / 4) % 2, cx + ox[i] + sx, cy + oy[i] + sy, &S);
  }
  char buf[64];
  snprintf(buf, sizeof buf, "HIGH SCORE %06ld", g->hiscore);
  text_center(r, buf, 150, 1.0f, 0.85f, 0.3f, 1);
  if ((frame / 30) % 2 == 0) text_center(r, "PRESS ENTER", 168, 1.0f, 1.0f, 1.0f, 1);
  text_center(r, "ESC FOR CONTROLS", 184, 0.55f, 0.65f, 0.9f, 1);
  text_center(r, "DOCK 8 MODULES - HUNT DINOS", 200, 0.55f, 0.85f, 0.6f, 1);
}

static void draw_controls(Renderer* r, long frame) {
  renderer_rect(r, 0, 0, SCREEN_W, SCREEN_H, 0, 0, 0.05f, 0.8f);
  text_center(r, "CONTROLS", 14, 1.0f, 0.9f, 0.3f, 1);
  static const char* const rows[][2] = {{"MOVE", "ARROWS / WASD"}, {"FIRE", "SPACE / Z"}, {"FORMATION", "X / LEFT SHIFT"}, {"PAUSE MENU", "ESC"}, {"START", "ENTER"},
                                        {"MUTE", "M"}, {"FULLSCREEN", "F"}, {"DEBUG", "F3"}};
  for (int i = 0; i < 8; i++) {
    renderer_text(r, rows[i][0], 24, 36 + (float)i * 11, 0.6f, 0.85f, 1.0f, 1);
    renderer_text(r, rows[i][1], 112, 36 + (float)i * 11, 1.0f, 1.0f, 1.0f, 1);
  }
  text_center(r, "SHOOT GROUND BUNKERS - THEY", 134, 0.9f, 0.9f, 0.9f, 1);
  text_center(r, "RELEASE NUMBERED MODULES", 144, 0.9f, 0.9f, 0.9f, 1);
  text_center(r, "FLY INTO THEM TO DOCK", 154, 0.9f, 0.9f, 0.9f, 1);
  text_center(r, "FILL THE GAUGE WITH KILLS", 168, 0.7f, 0.95f, 0.6f, 1);
  text_center(r, "THEN PRESS X WITH 2 OR MORE MODULES!", 178, 0.7f, 0.95f, 0.6f, 1);
  if ((frame / 30) % 2 == 0) text_center(r, "PRESS ENTER TO START", 204, 1.0f, 1.0f, 1.0f, 1);
}

static void draw_debug(Renderer* r, const Game* g) {
  const Player* p = &g->player;
  /* hitboxes */
  float x, y, hw, hh;
  player_hurtbox_core(g, &x, &y, &hw, &hh);
  renderer_rect(r, x - hw, y - hh, hw * 2, hh * 2, 0, 1, 0, 0.45f);
  for (int i = 0; i < NUM_COMPONENTS; i++)
    if (player_component_rect(g, i, &x, &y, &hw, &hh)) renderer_rect(r, x - hw, y - hh, hw * 2, hh * 2, 0, 0.8f, 1, 0.35f);
  POOL_EACH(const Enemy, en, g->enemies) {
    enemy_half_size(en->type, &hw, &hh);
    renderer_rect(r, en->pos.x - hw, en->pos.y - hh, hw * 2, hh * 2, 1, 0, 0, 0.35f);
  }
  POOL_EACH(const Ground, e, g->ground) {
    if (e->hp <= 0) continue;
    ground_half_size(e->type, &hw, &hh);
    float sy = sim_ground_sy(g, e->pos.y);
    renderer_rect(r, e->pos.x - hw, sy - hh, hw * 2, hh * 2, 1, 0.5f, 0, 0.35f);
  }
  POOL_EACH(const EBullet, eb, g->ebullets) renderer_rect(r, eb->pos.x - 2, eb->pos.y - 2, 4, 4, 1, 0, 1, 0.8f);
  if (g->boss.active) {
    for (int i = 0; i < 2; i++) { float gx, gy; boss_gun_pos(&g->boss, i, &gx, &gy); renderer_rect(r, gx - 8, gy - 8, 16, 16, 1, 1, 0, 0.3f); }
    float cx, cy; boss_core_pos(&g->boss, &cx, &cy); renderer_rect(r, cx - 8, cy - 8, 16, 16, 0, 1, 1, 0.3f);
  }
  /* counters */
  int ne = POOL_COUNT(g->enemies), ng = POOL_COUNT(g->ground), neb = POOL_COUNT(g->ebullets);
  int npb = POOL_COUNT(g->pbullets), nfx = POOL_COUNT(g->fx);
  char buf[96];
  renderer_rect(r, 0, 12, 150, 52, 0, 0, 0, 0.6f);
  snprintf(buf, sizeof buf, "E%d G%d EB%d PB%d FX%d", ne, ng, neb, npb, nfx);
  renderer_text_small(r, buf, 2, 13, 0.7f, 1, 0.7f);
  snprintf(buf, sizeof buf, "SCR%.0f/%.0f D%d SEED%u", g->scroll, g->stage_length, g->difficulty, g->seed);
  renderer_text_small(r, buf, 2, 22, 0.7f, 1, 0.7f);
  snprintf(buf, sizeof buf, "COMP%d%d%d%d FORM%d/%d G%d", p->comp[0], p->comp[1], p->comp[2], p->comp[3], p->form_state, p->form_t, p->gauge);
  renderer_text_small(r, buf, 2, 31, 0.7f, 1, 0.7f);
  snprintf(buf, sizeof buf, "INV%d PH%d TICK%ld", p->invuln, g->phase, g->tick);
  renderer_text_small(r, buf, 2, 40, 0.7f, 1, 0.7f);
  snprintf(buf, sizeof buf, "SPAWN %d/%d", g->spawn_next, g->spawn_count);
  renderer_text_small(r, buf, 2, 49, 0.7f, 1, 0.7f);
}

void scene_draw(Renderer* r, const Game* g, long frame) {
  if (g->map_rows > 0) {
    float sx = 0, sy = 0;
    if (g->shake > 0) { sx = (float)((frame * 7) % 3 - 1) * (g->shake > 8 ? 2.0f : 1.0f); sy = (float)((frame * 5) % 3 - 1); }
    renderer_translate(r, sx, sy);
    draw_terrain(r, g);
    draw_ground(r, g);
    draw_pickups(r, g, frame);
    draw_clouds(r, g);
    draw_enemies(r, g);
    draw_boss(r, g, frame);
    draw_player(r, g, frame);
    draw_bullets(r, g, frame);
    draw_fx(r, g);
    renderer_translate(r, 0, 0);
  }
  if (g->screen_flash > 0) renderer_rect(r, 0, 0, SCREEN_W, SCREEN_H, 1, 1, 1, g->screen_flash > 1 ? 1 : g->screen_flash * 0.8f);
  switch (g->mode) {
    case MODE_TITLE: draw_title(r, g, frame); break;
    case MODE_CONTROLS: draw_controls(r, frame); break;
    case MODE_GAMEOVER:
      draw_hud(r, g, frame);
      renderer_rect(r, 0, 70, SCREEN_W, 64, 0, 0, 0.05f, 0.7f);
      text_center(r, "GAME OVER", 84, 1.0f, 0.3f, 0.3f, 1);
      { char b[48]; snprintf(b, sizeof b, "SCORE %06ld", g->player.score); text_center(r, b, 102, 1, 1, 1, 1); }
      if (g->mode_tick > 90 && (frame / 30) % 2 == 0) text_center(r, "PRESS ENTER TO RESTART", 118, 1.0f, 0.9f, 0.3f, 1);
      break;
    case MODE_DEMO:
      draw_hud(r, g, frame);
      if ((frame / 30) % 2 == 0) text_center(r, "DEMO  PRESS ANY KEY", 40, 1.0f, 1.0f, 1.0f, 1);
      draw_banners(r, g, frame);
      break;
    case MODE_PLAYING:
      draw_hud(r, g, frame);
      draw_banners(r, g, frame);
      if (g->paused) draw_pause(r, g);
      break;
  }
  if (g->debug_overlay && g->map_rows > 0) draw_debug(r, g);
}
