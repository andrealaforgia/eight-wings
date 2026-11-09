#include "helpers.h"

/* ---- weapon types ---- */

static int count_kind(const Game* g, PBulletKind k) {
  int n = 0;
  POOL_EACH(const PBullet, b, g->pbullets) n += b->kind == k;
  return n;
}

static void fire_once(Game* g) {
  pool_reset(&g->pbullets);
  g->player.shot_cd = 0;
  tick_n(g, (Input){.fire = true}, 1);
}

static void test_each_weapon_has_its_own_shape_and_grows_per_tier(void) {
  static const int modules[5] = {0, 1, 3, 5, 7}; /* tiers 0..4 */
  static const struct { WeaponType w; PBulletKind kind; int per_tier[5]; } cases[] = {
      {WT_SPREAD, PB_PELLET, {3, 5, 7, 9, 11}},
      {WT_LASER, PB_LASER, {1, 2, 3, 3, 4}},
      {WT_MISSILE, PB_MISSILE, {2, 2, 4, 4, 6}},
      {WT_WAVE, PB_WAVE, {2, 2, 4, 4, 6}},
  };
  for (int c = 0; c < 4; c++)
    for (int t = 0; t < 5; t++) {
      Game* g = new_playing_game(3);
      g->player.weapon = cases[c].w;
      give_components(g, modules[t]);
      fire_once(g);
      CHECK_INT(count_kind(g, cases[c].kind), cases[c].per_tier[t]);
      CHECK_INT(count_active_pbullets(g), cases[c].per_tier[t]); /* and nothing of the old vulcan pattern */
      game_free(g);
      free(g);
    }
}

static void test_weapons_have_different_cadence(void) {
  Game* g = new_playing_game(3);
  g->player.weapon = WT_LASER;
  int laser = player_weapon_interval(g);
  g->player.weapon = WT_MISSILE;
  int missile = player_weapon_interval(g);
  g->player.weapon = WT_VULCAN;
  CHECK(laser < player_weapon_interval(g));
  CHECK(missile > player_weapon_interval(g));
  game_free(g);
  free(g);
}

static void test_weapon_capsule_switches_weapon_and_duplicate_pays(void) {
  Game* g = new_playing_game(3);
  CHECK_INT(g->player.weapon, WT_VULCAN);
  sim_pickup(g, -(WT_LASER + 1), g->player.pos.x, g->player.pos.y);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.weapon, WT_LASER);
  long s = g->player.score;
  sim_pickup(g, -(WT_LASER + 1), g->player.pos.x, g->player.pos.y);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.weapon, WT_LASER);
  CHECK(g->player.score >= s + 1000);
  game_free(g);
  free(g);
}

static void test_dying_resets_the_weapon(void) {
  Game* g = new_playing_game(3);
  g->player.weapon = WT_WAVE;
  g->player.invuln = 0;
  player_try_hit(g, g->player.pos.x, g->player.pos.y, 6);
  CHECK(g->player.state != PS_ALIVE);
  CHECK_INT(g->player.weapon, WT_VULCAN);
  game_free(g);
  free(g);
}

static void test_weapon_drop_never_repeats_the_current_weapon(void) {
  Game* g = new_playing_game(9);
  for (int w = 0; w < WT_COUNT; w++) {
    g->player.weapon = w;
    pool_reset(&g->pickups);
    for (int i = 0; i < 40; i++) sim_weapon_drop(g, 100, 50);
    POOL_EACH(const Pickup, k, g->pickups) CHECK(k->number < 0 && -k->number - 1 != w);
  }
  game_free(g);
  free(g);
}

static void test_missiles_curve_onto_a_target_beside_the_ship(void) {
  Game* g = new_playing_game(3);
  g->player.weapon = WT_MISSILE;
  g->god_mode = true;
  Enemy* e = enemy_spawn(g, ET_DIVER, 40, 60, 0, 1); /* a diver in its lock-on hold stays put */
  e->state = 1;
  e->timer = 100000;
  fire_once(g);
  bool killed = false;
  for (int t = 0; t < 120 && !killed; t++) {
    tick_n(g, (Input){0}, 1);
    killed = !POOL_ALIVE(g->enemies, e);
  }
  CHECK(killed);
  game_free(g);
  free(g);
}

static void test_wave_shots_snake_sideways(void) {
  Game* g = new_playing_game(3);
  g->player.weapon = WT_WAVE;
  fire_once(g);
  float lo = 1e9f, hi = -1e9f;
  for (int t = 0; t < 40; t++) {
    tick_n(g, (Input){0}, 1);
    POOL_EACH(const PBullet, b, g->pbullets) {
      if (b->pos.x < lo) lo = b->pos.x;
      if (b->pos.x > hi) hi = b->pos.x;
    }
  }
  CHECK(hi - lo > 14.0f);
  game_free(g);
  free(g);
}

static void test_spread_pellets_fade_but_laser_pierces(void) {
  Game* g = new_playing_game(3);
  g->player.weapon = WT_SPREAD;
  fire_once(g);
  tick_n(g, (Input){0}, 70);
  CHECK_INT(count_kind(g, PB_PELLET), 0); /* short range */
  g->player.weapon = WT_LASER;
  fire_once(g);
  PBullet* b = first_active(&g->pbullets);
  CHECK(b && b->pierce);
  game_free(g);
  free(g);
}

/* ---- new enemies ---- */

static void test_weaver_snakes_across_the_screen(void) {
  Game* g = new_playing_game(4);
  Enemy* e = enemy_spawn(g, ET_WEAVER, 128, 0, 0, 1);
  float lo = 1e9f, hi = -1e9f;
  for (int t = 0; t < 110; t++) {
    tick_n(g, (Input){0}, 1);
    if (!POOL_ALIVE(g->enemies, e)) break;
    if (e->pos.x < lo) lo = e->pos.x;
    if (e->pos.x > hi) hi = e->pos.x;
  }
  CHECK(hi - lo > 60.0f);
  game_free(g);
  free(g);
}

static void test_hunter_homes_on_the_player(void) {
  Game* g = new_playing_game(4);
  g->god_mode = true;
  g->player.pos.x = 220;
  Enemy* e = enemy_spawn(g, ET_HUNTER, 30, 0, 0, 1);
  tick_n(g, (Input){0}, 120);
  CHECK(POOL_ALIVE(g->enemies, e));
  CHECK(e->pos.x > 80); /* it swung across toward the player's column */
  game_free(g);
  free(g);
}

static void test_gunship_hovers_fires_and_drops_a_weapon(void) {
  Game* g = new_playing_game(4);
  g->god_mode = true;
  Enemy* e = enemy_spawn(g, ET_GUNSHIP, 128, 0, 0, 1);
  int fired = 0;
  for (int t = 0; t < 200; t++) {
    tick_n(g, (Input){0}, 1);
    fired = fired || pool_get_active_count(&g->ebullets) >= 4;
  }
  CHECK(fired);
  CHECK(POOL_ALIVE(g->enemies, e));
  CHECK(e->pos.y > 30 && e->pos.y < 80); /* hovering overhead */
  enemy_hit(g, e, 999);
  int weapons = 0;
  POOL_EACH(const Pickup, k, g->pickups) weapons += k->number < 0;
  CHECK_INT(weapons, 1);
  game_free(g);
  free(g);
}

static void test_splitter_bursts_into_three(void) {
  Game* g = new_playing_game(4);
  Enemy* e = enemy_spawn(g, ET_SPLITTER, 128, 80, 0, 1);
  enemy_hit(g, e, 999);
  CHECK_INT(pool_get_active_count(&g->enemies), 3);
  game_free(g);
  free(g);
}

/* ---- big dinosaurs ---- */

static float world_y_for_screen(const Game* g, float sy) { return SCREEN_H + g->scroll - sy; }

static void test_big_dinos_leave_bones_and_a_weapon(void) {
  static const GroundType big[3] = {GT_TRIKE, GT_SPINO, GT_BRONTO};
  for (int i = 0; i < 3; i++) {
    Game* g = new_playing_game(6);
    Ground* d = ground_spawn(g, big[i], 128, world_y_for_screen(g, 90), 0);
    CHECK(ground_is_dino(big[i]));
    CHECK(d->hp >= 24);
    float hw, hh;
    ground_half_size(big[i], &hw, &hh);
    CHECK(hw >= 17);
    ground_hit(g, d, 9999);
    CHECK(POOL_ALIVE(g->ground, d) && d->hp <= 0); /* the skeleton stays */
    int weapons = 0;
    POOL_EACH(const Pickup, k, g->pickups) weapons += k->number < 0;
    CHECK_INT(weapons, 1);
    game_free(g);
    free(g);
  }
}

static void test_triceratops_winds_up_then_charges(void) {
  Game* g = new_playing_game(6);
  g->god_mode = true;
  Ground* d = ground_spawn(g, GT_TRIKE, g->player.pos.x, world_y_for_screen(g, 60), 0);
  d->age = 100;
  bool windup = false, charge = false;
  float y0 = d->pos.y;
  float charge_speed = 0;
  for (int t = 0; t < 260 && !charge; t++) {
    float before = d->pos.y;
    tick_n(g, (Input){0}, 1);
    windup = windup || d->state == 1;
    if (d->state == 2) { charge = true; charge_speed = fabsf(d->pos.y - before); }
    (void)y0;
  }
  CHECK(windup);
  CHECK(charge);
  (void)charge_speed;
  game_free(g);
  free(g);
}

static void test_bronto_stomp_sends_out_a_ring(void) {
  Game* g = new_playing_game(6);
  g->god_mode = true;
  Ground* d = ground_spawn(g, GT_BRONTO, 128, world_y_for_screen(g, 90), 0);
  d->timer = 2;
  tick_n(g, (Input){0}, 4);
  CHECK(pool_get_active_count(&g->ebullets) >= 14);
  game_free(g);
  free(g);
}

static void test_spino_spits_a_volley(void) {
  Game* g = new_playing_game(6);
  g->god_mode = true;
  Ground* d = ground_spawn(g, GT_SPINO, 128, world_y_for_screen(g, 80), 0);
  d->timer = 2;
  tick_n(g, (Input){0}, 4);
  CHECK(pool_get_active_count(&g->ebullets) >= 5);
  game_free(g);
  free(g);
}

/* ---- the level script uses all of it ---- */

static void test_every_stage_script_contains_the_new_variety(void) {
  for (int stage = 0; stage < NUM_STAGES; stage++) {
    Game* g = (Game*)calloc(1, sizeof(Game));
    game_init(g, 1985);
    game_new_run(g, stage);
    bool air[ET_COUNT] = {0}, ground[GT_COUNT] = {0};
    for (int i = 0; i < g->spawn_count; i++) {
      if (g->spawns[i].kind == SP_AIR) air[g->spawns[i].type] = true;
      else ground[g->spawns[i].type] = true;
    }
    int new_air = air[ET_WEAVER] + air[ET_HUNTER] + air[ET_GUNSHIP] + air[ET_SPLITTER];
    int big = ground[GT_TRIKE] + ground[GT_SPINO] + ground[GT_BRONTO];
    printf("    stage %d: new flyer kinds=%d big dino kinds=%d spawns=%d\n", stage, new_air, big, g->spawn_count);
    CHECK(new_air >= 3);
    CHECK(big >= 2);
    game_free(g);
    free(g);
  }
}

int main(void) {
  printf("test_variety\n");
  RUN(test_each_weapon_has_its_own_shape_and_grows_per_tier);
  RUN(test_weapons_have_different_cadence);
  RUN(test_weapon_capsule_switches_weapon_and_duplicate_pays);
  RUN(test_dying_resets_the_weapon);
  RUN(test_weapon_drop_never_repeats_the_current_weapon);
  RUN(test_missiles_curve_onto_a_target_beside_the_ship);
  RUN(test_wave_shots_snake_sideways);
  RUN(test_spread_pellets_fade_but_laser_pierces);
  RUN(test_weaver_snakes_across_the_screen);
  RUN(test_hunter_homes_on_the_player);
  RUN(test_gunship_hovers_fires_and_drops_a_weapon);
  RUN(test_splitter_bursts_into_three);
  RUN(test_big_dinos_leave_bones_and_a_weapon);
  RUN(test_triceratops_winds_up_then_charges);
  RUN(test_bronto_stomp_sends_out_a_ring);
  RUN(test_spino_spits_a_volley);
  RUN(test_every_stage_script_contains_the_new_variety);
  return test_report();
}
