/* Level script (what spawns when) and the procedural terrain map. All deterministic from the game seed. */
#include <stdlib.h>
#include <string.h>

#include "game/sim.h"

/* local tile ids within a theme (sprite names tile_t<T>_...) */
enum { L_BASE = 0, L_DETAIL = 4, L_FOREST = 8, L_STRUCT = 10, L_ROAD_V = 14, L_ROAD_H = 15, L_ROAD_X = 16, L_LIQ = 17 };

int level_theme(int stage) { return stage % NUM_STAGES; }
float level_length(int stage) { return 3800.0f + 350.0f * (float)stage; }
int level_tile_sprite_local(uint16_t tile) { return tile % TILES_PER_THEME; }

uint16_t level_tile(const Game* g, int row, int col) {
  if (col < 0) col = 0;
  if (col >= MAP_COLS) col = MAP_COLS - 1;
  if (row < 0) row = 0;
  if (row >= g->map_rows) row = g->map_rows - 1;
  return g->map[row][col];
}

/* ---------------- spawn script ---------------- */

static int cmp_spawn(const void* a, const void* b) {
  float x = ((const Spawn*)a)->at, y = ((const Spawn*)b)->at;
  return (x > y) - (x < y);
}

static void add_spawn(Game* g, Spawn s) {
  if (g->spawn_count < MAX_SPAWNS) g->spawns[g->spawn_count++] = s;
}
static void add_air(Game* g, float at, EnemyType t, float x, int y_off, float p0, int dir) {
  add_spawn(g, (Spawn){at, SP_AIR, (int)t, x, p0, dir, y_off});
}
static void add_ground(Game* g, GroundType t, float wx, float wy, float p0) {
  add_spawn(g, (Spawn){wy - SCREEN_H - 20.0f, SP_GROUND, (int)t, wx, wy, (int)p0, 0});
  (void)p0;
}

static float rf(rng_t* r, float lo, float hi) { return lo + (hi - lo) * rng_float(r); }

static void wave(Game* g, rng_t* r, int stage, float d, int kind) {
  float cx = rf(r, 48, 208);
  int side = rng_chance(r, 0.5f) ? 1 : -1;
  switch (kind) {
    case 0: /* snake of drones */
      for (int i = 0; i < 5 + stage; i++) add_air(g, d + (float)i * 15.0f, ET_DRONE, cx, 0, 0.0f, 1);
      break;
    case 1: /* swooping arc from one side */
      for (int i = 0; i < 4 + stage; i++) add_air(g, d + (float)i * 14.0f, ET_SWOOP, side > 0 ? -10.0f : SCREEN_W + 10.0f, 0, 0, side);
      break;
    case 2: { /* V formation of wingmen that peel off */
      static const int ox[5] = {0, -16, 16, -32, 32}, oy[5] = {0, -12, -12, -24, -24};
      for (int i = 0; i < 5; i++) add_air(g, d, ET_WING, cx + (float)ox[i], oy[i], 0, (ox[i] < 0) ? -1 : 1);
      break;
    }
    case 3: /* divers */
      for (int i = 0; i < 2 + stage; i++) add_air(g, d + (float)i * 22.0f, ET_DIVER, rf(r, 30, 226), 0, rf(r, 0, 30), 1);
      break;
    case 4: /* spinner pair */
      add_air(g, d, ET_SPINNER, 70, 0, 0, 1);
      add_air(g, d + 18, ET_SPINNER, 186, 0, 3.14159f, 1);
      break;
    case 5: /* zigzag saucers */
      add_air(g, d, ET_SAUCER, rf(r, 60, 196), 0, 0, 1);
      if (stage >= 1) add_air(g, d + 40, ET_SAUCER, rf(r, 60, 196), 0, 0, 1);
      break;
    default: /* bomber */
      add_air(g, d, ET_BOMBER, rf(r, 60, 196), 0, 0, 1);
      break;
  }
}

static void script_air(Game* g, rng_t* r, int stage, float len) {
  /* weights per kind: drone swoop wing diver spinner saucer bomber */
  static const int W[NUM_STAGES][7] = {{30, 25, 25, 12, 5, 5, 0}, {15, 20, 18, 15, 10, 12, 10}, {10, 14, 15, 20, 14, 15, 12}};
  float d = 260;
  while (d < len - 220) {
    int total = 0;
    for (int k = 0; k < 7; k++) total += W[stage][k];
    int pick = rng_range(r, 0, total - 1), kind = 0;
    for (; kind < 7; kind++) {
      if (pick < W[stage][kind]) break;
      pick -= W[stage][kind];
    }
    if (kind > 6) kind = 0;
    /* introduce harder kinds a little later in the first stage */
    if (stage == 0 && d < 900 && kind >= 3) kind = (int)(d / 150) % 3;
    wave(g, r, stage, d, kind);
    d += rf(r, 120.0f, 190.0f) - 26.0f * (float)stage;
  }
}

static void script_ground(Game* g, rng_t* r, int stage, float len) {
  /* guaranteed supply of numbered component bunkers, escorted by turrets */
  float y = 330;
  int bunker_n = 0;
  while (y < len + 100) {
    float x = rf(r, 40, 216);
    add_ground(g, GT_BUNKER, x, y, 0);
    bunker_n++;
    if (stage > 0 || bunker_n > 1) {
      add_ground(g, GT_TURRET, x - 34, y + rf(r, -10, 26), 0);
      add_ground(g, GT_TURRET, x + 34, y + rf(r, -10, 26), 0);
    }
    y += rf(r, 330, 440);
  }
  /* scatter of other targets */
  y = 420;
  while (y < len + 50) {
    int pick = rng_range(r, 0, 99);
    float x = rf(r, 24, 232);
    if (pick < 14) {
      for (int k = 0; k < 2 + stage; k++) add_ground(g, GT_RAPTOR, x + (float)(k * 22 - 22), y + (float)(k * 12), 0); /* a hunting pack */
    } else if (pick < 22 && (stage >= 1 || y > 1500)) {
      add_ground(g, GT_REX, x, y, 0);
    } else if (pick < 34) add_ground(g, GT_TURRET, x, y, 0);
    else if (pick < 50) add_ground(g, GT_TANK, x, y, rng_chance(r, 0.5f) ? 1 : -1);
    else if (pick < 68) add_ground(g, GT_RADAR, x, y, 0);
    else if (pick < 86 || stage == 0) add_ground(g, GT_SILO, x, y, 0);
    else add_ground(g, GT_FORT, x, y, 0);
    if (rng_chance(r, 0.3f)) add_ground(g, GT_TURRET, x + (rng_chance(r, 0.5f) ? 28 : -28), y + 14, 0);
    y += rf(r, 75, 140) - 12.0f * (float)stage;
  }
  /* every stage has at least one stomping rex and a raptor pack early on */
  add_ground(g, GT_RAPTOR, 100, 640, 0); add_ground(g, GT_RAPTOR, 140, 664, 0); add_ground(g, GT_RAPTOR, 120, 700, 0);
  add_ground(g, GT_REX, 128, len * 0.35f, 0);
  /* one showpiece fortress per stage */
  add_ground(g, GT_FORT, 128, len * 0.55f, 0);
  add_ground(g, GT_TURRET, 90, len * 0.55f + 30, 0);
  add_ground(g, GT_TURRET, 166, len * 0.55f + 30, 0);
}

/* ---------------- terrain ---------------- */

static uint32_t hash2(uint32_t a, uint32_t b, uint32_t seed) {
  uint32_t h = a * 374761393u + b * 668265263u + seed * 2246822519u;
  h = (h ^ (h >> 13)) * 1274126177u;
  return h ^ (h >> 16);
}

static float value_noise(float x, float y, uint32_t seed) {
  int ix = (int)floorf(x), iy = (int)floorf(y);
  float fx = x - (float)ix, fy = y - (float)iy;
  fx = fx * fx * (3 - 2 * fx);
  fy = fy * fy * (3 - 2 * fy);
  float v00 = (float)(hash2((uint32_t)ix, (uint32_t)iy, seed) & 1023) / 1023.0f;
  float v10 = (float)(hash2((uint32_t)ix + 1, (uint32_t)iy, seed) & 1023) / 1023.0f;
  float v01 = (float)(hash2((uint32_t)ix, (uint32_t)iy + 1, seed) & 1023) / 1023.0f;
  float v11 = (float)(hash2((uint32_t)ix + 1, (uint32_t)iy + 1, seed) & 1023) / 1023.0f;
  return (v00 + (v10 - v00) * fx) * (1 - fy) + (v01 + (v11 - v01) * fx) * fy;
}

#define MAXR MAP_MAX_ROWS
static bool s_liquid[MAXR][MAP_COLS];
static bool s_forced_land[MAXR][MAP_COLS];
static int s_road[MAXR][MAP_COLS]; /* 0 none, 1 v, 2 h, 3 x */

static void build_terrain(Game* g, int stage, rng_t* r) {
  int theme = level_theme(stage);
  uint32_t seed = g->seed * 31u + (uint32_t)stage * 7919u;
  int rows = (int)((level_length(stage) + 1700.0f) / TILE) + 24;
  if (rows > MAXR) rows = MAXR;
  g->map_rows = rows;
  memset(s_forced_land, 0, sizeof s_forced_land);
  memset(s_road, 0, sizeof s_road);
  memset(s_liquid, 0, sizeof s_liquid);

  /* forced land under every ground target so fortresses never float on the sea */
  for (int i = 0; i < g->spawn_count; i++) {
    const Spawn* s = &g->spawns[i];
    if (s->kind != SP_GROUND) continue;
    int col = (int)(s->x / TILE), row = (int)(s->p0 / TILE);
    int rad = (s->type == GT_FORT || s->type == GT_REX) ? 3 : 2;
    for (int dy = -rad; dy <= rad; dy++)
      for (int dx = -rad; dx <= rad; dx++) {
        int rr = row + dy, cc = col + dx;
        if (rr >= 0 && rr < rows && cc >= 0 && cc < MAP_COLS) s_forced_land[rr][cc] = true;
      }
  }
  /* liquid field */
  static const float thr[3] = {0.80f, 0.47f, 0.84f};
  static const float scale[3] = {0.11f, 0.075f, 0.12f};
  for (int rr = 0; rr < rows; rr++)
    for (int cc = 0; cc < MAP_COLS; cc++) {
      float n = value_noise((float)cc * scale[theme] * 2.2f, (float)rr * scale[theme], seed) * 0.7f +
                value_noise((float)cc * scale[theme] * 5.0f, (float)rr * scale[theme] * 2.3f, seed ^ 0x55) * 0.3f;
      s_liquid[rr][cc] = n > thr[theme] && !s_forced_land[rr][cc] && rr > 6;
    }
  /* roads: a few long vertical runs and cross streets */
  int vcols[2] = {rng_range(r, 2, 5), rng_range(r, 10, 13)};
  for (int v = 0; v < 2; v++) {
    int c = vcols[v], rr = 8 + v * 40;
    while (rr < rows - 8) {
      int run = rng_range(r, 25, 70), gap = rng_range(r, 20, 60);
      if (theme == 1 && v == 1) gap += 30; /* fewer causeways at sea */
      for (int k = 0; k < run && rr < rows; k++, rr++) s_road[rr][c] |= 1;
      rr += gap;
    }
  }
  for (int rr = 20; rr < rows - 4; rr += rng_range(r, 28, 46)) {
    int c0 = rng_range(r, 0, 4), c1 = rng_range(r, 11, 15);
    if (theme == 1 && rng_chance(r, 0.6f)) continue;
    for (int c = c0; c <= c1; c++) s_road[rr][c] |= 2;
  }
  /* compose tiles */
  for (int rr = 0; rr < rows; rr++) {
    for (int cc = 0; cc < MAP_COLS; cc++) {
      uint32_t h = hash2((uint32_t)cc, (uint32_t)rr, seed);
      int local;
      if (s_road[rr][cc]) {
        local = s_road[rr][cc] == 1 ? L_ROAD_V : (s_road[rr][cc] == 2 ? L_ROAD_H : L_ROAD_X);
      } else if (s_liquid[rr][cc]) {
        int mask = 0;
        int nr[4] = {rr + 1, rr, rr - 1, rr}, nc[4] = {cc, cc + 1, cc, cc - 1};
        for (int k = 0; k < 4; k++) {
          int r2 = nr[k], c2 = nc[k];
          bool land;
          if (c2 < 0 || c2 >= MAP_COLS || r2 < 0 || r2 >= rows) land = false;
          else land = !s_liquid[r2][c2] || s_road[r2][c2];
          if (land) mask |= (1 << k);
        }
        local = L_LIQ + mask; /* bit0 = north(row+1 is further ahead = screen-up), 1 east, 2 south, 3 west */
      } else {
        float cluster = value_noise((float)cc * 0.35f, (float)rr * 0.35f, seed ^ 0x77);
        bool near_target = s_forced_land[rr][cc];
        uint32_t pick = h % 100;
        if (!near_target && cluster > 0.74f && pick < 70) local = L_FOREST + (int)(h >> 8) % 2;
        else if (!near_target && pick < 3) local = L_STRUCT + (int)(h >> 8) % 4;
        else if (pick < 20) local = L_DETAIL + (int)(h >> 8) % 4;
        else local = L_BASE + (int)(h >> 8) % 4;
      }
      g->map[rr][cc] = (uint16_t)(theme * TILES_PER_THEME + local);
    }
  }
}

void level_build(Game* g, int stage) {
  rng_t r;
  rng_seed(&r, g->seed * 2654435761u + (uint32_t)stage * 97u + 13u);
  g->spawn_count = 0;
  g->spawn_next = 0;
  float len = level_length(stage);
  script_air(g, &r, stage, len);
  script_ground(g, &r, stage, len);
  qsort(g->spawns, (size_t)g->spawn_count, sizeof(Spawn), cmp_spawn);
  build_terrain(g, stage, &r);
}

void level_update(Game* g) {
  float target = g->scroll_speed;
  float speed = target;
  if (g->phase == PH_BOSS) speed = target * 0.45f;
  else if (g->phase == PH_CLEAR) speed = target * (1.0f + (float)g->phase_tick / 60.0f);
  g->scroll += speed;
  while (g->spawn_next < g->spawn_count && g->spawns[g->spawn_next].at <= g->scroll) {
    const Spawn* s = &g->spawns[g->spawn_next++];
    if (g->phase == PH_BOSS || g->phase == PH_CLEAR) continue;
    if (s->kind == SP_AIR) enemy_spawn(g, (EnemyType)s->type, s->x, -12.0f + (float)s->extra, s->p0, s->dir);
    else ground_spawn(g, (GroundType)s->type, s->x, s->p0, (float)s->dir);
  }
}
