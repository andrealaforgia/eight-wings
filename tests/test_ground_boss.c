#include "helpers.h"

static void test_bunker_releases_next_missing_component(void) {
  Game* g = new_playing_game(7);
  Ground* b = ground_spawn(g, GT_BUNKER, 100, sim_ground_wy(g, 80), 0);
  ground_hit(g, b, 99);
  CHECK(b->hp <= 0);
  Pickup* first = (Pickup*)first_active(&g->pickups);
  CHECK(first != NULL);
  CHECK_INT(first->number, 1);
  /* second bunker: pod 1 is still pending, so it releases pod 2 */
  Ground* b2 = ground_spawn(g, GT_BUNKER, 140, sim_ground_wy(g, 60), 0);
  ground_hit(g, b2, 99);
  CHECK_INT(POOL_COUNT(g->pickups), 2);
  CHECK_INT(((Pickup*)pool_get_at(&g->pickups, pool_index_of(&g->pickups, first) == 0 ? 1 : 0))->number + first->number, 3);
  game_free(g);
  free(g);
}

static void test_bunker_after_full_assembly_gives_medal(void) {
  Game* g = new_playing_game(7);
  give_components(g, NUM_COMPONENTS);
  Ground* b = ground_spawn(g, GT_BUNKER, 100, sim_ground_wy(g, 80), 0);
  ground_hit(g, b, 99);
  CHECK_INT(((Pickup*)first_active(&g->pickups))->number, 0);
  game_free(g);
  free(g);
}

static void test_lost_component_can_be_recovered(void) {
  Game* g = new_playing_game(7);
  give_components(g, 4);
  player_lose_component(g, 1);
  CHECK_INT(sim_next_missing_component(g), 2);
  game_free(g);
  free(g);
}

static void test_ground_scrolls_with_terrain(void) {
  Game* g = new_playing_game(7);
  Ground* e = ground_spawn(g, GT_RADAR, 100, sim_ground_wy(g, 50), 0);
  float y0 = sim_ground_sy(g, e->pos.y);
  tick_n(g, (Input){0}, 20);
  float y1 = sim_ground_sy(g, e->pos.y);
  CHECK_NEAR(y1 - y0, g->scroll_speed * 20, 0.01);
  game_free(g);
  free(g);
}

static void test_turret_aims_toward_player(void) {
  Game* g = new_playing_game(7);
  g->player.invuln = 99999;
  g->player.pos = (Vec2){200, 100};
  Ground* e = ground_spawn(g, GT_TURRET, 100, sim_ground_wy(g, 100), 0);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(e->aim, 2); /* due east */
  g->player.pos = (Vec2){100, 20};
  tick_n(g, (Input){0}, 1);
  CHECK_INT(e->aim, 0); /* up */
  game_free(g);
  free(g);
}

static void test_rubble_does_not_collide(void) {
  Game* g = new_playing_game(7);
  Ground* e = ground_spawn(g, GT_TURRET, 100, sim_ground_wy(g, 100), 0);
  ground_hit(g, e, 99);
  long s = g->player.score;
  shoot_at(g, 100, 100, 1);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.score, s);
  game_free(g);
  free(g);
}

/* ---- dinosaurs ---- */

static void test_raptor_hunts_player_and_leaves_bones(void) {
  Game* g = new_playing_game(11);
  g->player.invuln = 99999;
  g->player.pos = (Vec2){200, 150};
  Ground* d = ground_spawn(g, GT_RAPTOR, 40, sim_ground_wy(g, 100), 0);
  float x0 = d->pos.x;
  for (int i = 0; i < 90; i++) { tick_n(g, (Input){0}, 1); g->player.pos = (Vec2){200, 150}; }
  CHECK(d->pos.x > x0 + 20);           /* it walked toward the player's column */
  CHECK(d->p0 > 0.2f);                 /* heading turned to the right */
  long s = g->player.score;
  ground_hit(g, d, 99);
  CHECK(d->hp <= 0 && POOL_ALIVE(g->ground, d));      /* the skeleton stays on the ground ... */
  CHECK_INT(g->player.score - s, sim_kill_points(g, ground_score(GT_RAPTOR)));
  float bx = d->pos.x, by = d->pos.y, h = d->p0;
  float sy0 = sim_ground_sy(g, d->pos.y);
  tick_n(g, (Input){0}, 30);
  CHECK_NEAR(d->pos.x, bx, 0.0001);    /* ... does not walk any more ... */
  CHECK_NEAR(d->pos.y, by, 0.0001);
  CHECK_NEAR(d->p0, h, 0.0001);
  CHECK_NEAR(sim_ground_sy(g, d->pos.y) - sy0, g->scroll_speed * 30, 0.01); /* ... and scrolls with the terrain */
  /* bones cannot be shot or collided with */
  long s2 = g->player.score;
  shoot_at(g, d->pos.x, sim_ground_sy(g, d->pos.y), 5);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.score, s2);
  game_free(g);
  free(g);
}

static void test_rex_is_tough_spits_fire_and_drops_medal(void) {
  Game* g = new_playing_game(11);
  g->player.invuln = 99999;
  Ground* d = ground_spawn(g, GT_REX, 128, sim_ground_wy(g, 60), 0);
  CHECK(d->hp >= 10);
  int seen = 0;
  d->timer = 1;
  for (int i = 0; i < 120; i++) { tick_n(g, (Input){0}, 1); seen += POOL_COUNT(g->ebullets); pool_reset(&g->ebullets); }
  CHECK(seen > 0);
  ground_hit(g, d, 999);
  int medals = 0;
  POOL_EACH(Pickup, k, g->pickups) medals += (k->number == 0);
  CHECK_INT(medals, 1);
  game_free(g);
  free(g);
}

static void test_dinosaurs_do_not_wade_into_water(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1985);
  game_new_run(g, 1); /* the archipelago stage */
  g->spawn_count = 0;
  g->player.invuln = 99999;
  int violations = 0, walked = 0;
  bool was_liquid[MAX_GROUND] = {false};
  for (int pack = 0; pack < 12; pack++) {
    float wy = 600.0f + (float)pack * 200.0f;
    ground_spawn(g, GT_RAPTOR, 60.0f + (float)(pack * 37 % 130), wy, 0);
  }
  for (int t = 0; t < 1800; t++) {
    g->player.pos = (Vec2){60.0f + (float)(t / 4 % 130), 150};
    g->scroll += 0.0f;
    game_tick(g, &(Input){0});
    POOL_EACH(Ground, d, g->ground) {
      size_t i = pool_index_of(&g->ground, d);
      if (d->hp <= 0) continue;
      int local = level_tile_sprite_local(level_tile(g, (int)(d->pos.y / TILE), (int)(d->pos.x / TILE)));
      bool liquid = local >= 17 && local <= 32;
      if (liquid && !was_liquid[i] && d->age > 1) violations++; /* stepped from land into water */
      was_liquid[i] = liquid;
      walked++;
    }
  }
  CHECK(walked > 100);
  CHECK_INT(violations, 0); /* walkers turn away from water */
  game_free(g);
  free(g);
}

/* ---- bosses ---- */

static void start_boss(Game* g, int stage) {
  g->stage = stage;
  boss_start(g);
  g->phase = PH_BOSS;
  g->player.invuln = 99999;
  tick_n(g, (Input){0}, 140);
}

static void test_boss_phases(void) {
  for (int st = 0; st < 3; st++) {
    Game* g = new_playing_game(9);
    start_boss(g, st);
    Boss* b = &g->boss;
    CHECK(b->active && b->entering == 0);
    CHECK_INT(b->phase, 0);
    float cx, cy;
    boss_core_pos(b, &cx, &cy);
    /* phase 0: core is shielded */
    int hp = b->core_hp;
    shoot_at(g, cx, cy, 5);
    tick_n(g, (Input){0}, 1);
    CHECK_INT(b->core_hp, hp);
    /* destroy both guns */
    for (int i = 0; i < 2; i++) {
      float gx, gy;
      boss_gun_pos(b, i, &gx, &gy);
      for (int k = 0; k < 20 && b->gun_hp[i] > 0; k++) {
        boss_gun_pos(b, i, &gx, &gy);
        shoot_at(g, gx, gy, 20);
        tick_n(g, (Input){0}, 1);
      }
    }
    CHECK_INT(b->gun_hp[0], 0);
    CHECK_INT(b->gun_hp[1], 0);
    CHECK_INT(b->phase, 1);
    /* core takes damage now, then enrages below 40% */
    boss_core_pos(b, &cx, &cy);
    shoot_at(g, cx, cy, 1);
    tick_n(g, (Input){0}, 1);
    CHECK_INT(b->core_hp, b->core_max - 1);
    b->core_hp = b->core_max * 4 / 10 + 1;
    boss_core_pos(b, &cx, &cy);
    shoot_at(g, cx, cy, 2);
    tick_n(g, (Input){0}, 1);
    CHECK_INT(b->phase, 2);
    /* kill */
    boss_core_pos(b, &cx, &cy);
    shoot_at(g, cx, cy, 999);
    tick_n(g, (Input){0}, 1);
    CHECK(b->dying > 0);
    tick_n(g, (Input){0}, 200);
    CHECK(!b->active);
    game_free(g);
  free(g);
  }
}

static void test_boss_attacks_fire_bullets(void) {
  for (int st = 0; st < 3; st++) {
    Game* g = new_playing_game(9);
    start_boss(g, st);
    int seen = 0;
    for (int i = 0; i < 240; i++) {
      tick_n(g, (Input){0}, 1);
      seen += POOL_COUNT(g->ebullets);
      pool_reset(&g->ebullets);
    }
    CHECK(seen > 10);
    game_free(g);
  free(g);
  }
}

static void test_boss_deterministic(void) {
  long a[2];
  for (int run = 0; run < 2; run++) {
    Game* g = new_playing_game(9);
    start_boss(g, 1);
    g->boss.phase = 1;
    g->boss.gun_hp[0] = g->boss.gun_hp[1] = 0;
    tick_n(g, (Input){0}, 300);
    long h = 0;
    POOL_EACH(EBullet, eb, g->ebullets) h += (long)(eb->pos.x * 7 + eb->pos.y * 13);
    a[run] = h + (long)(g->boss.pos.x * 100);
    game_free(g);
  free(g);
  }
  CHECK_INT(a[0], a[1]);
}

static void test_beam_telegraph_then_damage(void) {
  Game* g = new_playing_game(9);
  start_boss(g, 1);
  g->player.invuln = 0;
  g->player.pos = (Vec2){60, 200};
  g->boss.beams[0] = (Beam){60, 10, 20};
  tick_n(g, (Input){0}, 5);
  CHECK_INT(g->player.state, PS_ALIVE); /* warning only */
  tick_n(g, (Input){0}, 8);
  CHECK_INT(g->player.state, PS_DEAD);
  game_free(g);
  free(g);
}

static void test_boss_defeat_clears_stage(void) {
  Game* g = new_playing_game(9);
  g->scroll = g->stage_length;
  tick_n(g, (Input){0}, 2);
  CHECK_INT(g->phase, PH_BOSS);
  g->player.invuln = 99999;
  tick_n(g, (Input){0}, 140);
  g->boss.dying = 1;
  tick_n(g, (Input){0}, 200);
  CHECK_INT(g->phase, PH_CLEAR);
  CHECK_INT(g->stats_bosses, 1);
  tick_n(g, (Input){0}, STAGE_CLEAR_TICKS + 2);
  CHECK_INT(g->stage, 1);
  CHECK_INT(g->phase, PH_INTRO);
  game_free(g);
  free(g);
}

int main(void) {
  printf("test_ground_boss\n");
  RUN(test_bunker_releases_next_missing_component);
  RUN(test_bunker_after_full_assembly_gives_medal);
  RUN(test_lost_component_can_be_recovered);
  RUN(test_ground_scrolls_with_terrain);
  RUN(test_turret_aims_toward_player);
  RUN(test_rubble_does_not_collide);
  RUN(test_raptor_hunts_player_and_leaves_bones);
  RUN(test_rex_is_tough_spits_fire_and_drops_medal);
  RUN(test_dinosaurs_do_not_wade_into_water);
  RUN(test_boss_phases);
  RUN(test_boss_attacks_fire_bullets);
  RUN(test_boss_deterministic);
  RUN(test_beam_telegraph_then_damage);
  RUN(test_boss_defeat_clears_stage);
  return test_report();
}
