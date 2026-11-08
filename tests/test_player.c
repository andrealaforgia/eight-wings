#include "helpers.h"

static void test_eight_direction_movement(void) {
  Game* g = new_playing_game(1);
  Vec2 s = g->player.pos;
  tick_n(g, (Input){.right = true}, 10);
  CHECK_NEAR(g->player.pos.x - s.x, PLAYER_SPEED * 10, 0.01);
  CHECK_NEAR(g->player.pos.y, s.y, 0.01);
  s = g->player.pos;
  tick_n(g, (Input){.up = true, .left = true}, 10);
  double dx = s.x - g->player.pos.x, dy = s.y - g->player.pos.y;
  CHECK_NEAR(dx, dy, 0.01);
  CHECK_NEAR(sqrt(dx * dx + dy * dy), PLAYER_SPEED * 10, 0.05); /* diagonal is normalised, not faster */
  game_free(g);
  free(g);
}

static void test_screen_bounds(void) {
  Game* g = new_playing_game(1);
  tick_n(g, (Input){.left = true, .up = true}, 400);
  CHECK_NEAR(g->player.pos.x, PLAYER_MARGIN_X, 0.001);
  CHECK_NEAR(g->player.pos.y, PLAYER_TOP, 0.001);
  tick_n(g, (Input){.right = true, .down = true}, 600);
  CHECK_NEAR(g->player.pos.x, SCREEN_W - PLAYER_MARGIN_X, 0.001);
  CHECK_NEAR(g->player.pos.y, PLAYER_BOTTOM, 0.001);
  game_free(g);
  free(g);
}

static void test_shooting_cadence_and_simultaneous_move_fire(void) {
  Game* g = new_playing_game(1);
  tick_n(g, (Input){.fire = true, .right = true}, 1);
  CHECK_INT(count_active_pbullets(g), 1); /* first shot immediately */
  tick_n(g, (Input){.fire = true, .right = true}, SHOT_INTERVAL * 3 - 1);
  CHECK_INT(count_active_pbullets(g), 3); /* volleys at ticks 0, 7, 14 */
  CHECK(g->player.pos.x > PLAYER_SPAWN_X); /* movement not dropped while firing */
  game_free(g);
  free(g);
}

static void test_bullets_travel_up_and_expire(void) {
  Game* g = new_playing_game(1);
  tick_n(g, (Input){.fire = true}, 1);
  float y0 = ((PBullet*)first_active(&g->pbullets))->pos.y;
  tick_n(g, (Input){0}, 5);
  CHECK(((PBullet*)first_active(&g->pbullets))->pos.y < y0 - 20);
  tick_n(g, (Input){0}, 120);
  CHECK_INT(count_active_pbullets(g), 0);
  game_free(g);
  free(g);
}

static void test_core_hit_kills_player_and_respawns_invulnerable(void) {
  Game* g = new_playing_game(1);
  int lives = g->player.lives;
  sim_ebullet(g, EB_ORB, g->player.pos.x, g->player.pos.y, 0, 0);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.state, PS_DEAD);
  CHECK_INT(g->player.lives, lives - 1);
  tick_n(g, (Input){0}, RESPAWN_DELAY + 2);
  CHECK_INT(g->player.state, PS_ALIVE);
  CHECK(g->player.invuln > 0);
  /* invulnerable: a bullet on top of the ship does nothing */
  sim_ebullet(g, EB_ORB, g->player.pos.x, g->player.pos.y, 0, 0);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.state, PS_ALIVE);
  /* ...until invulnerability runs out */
  tick_n(g, (Input){0}, RESPAWN_INVULN + 1);
  sim_ebullet(g, EB_ORB, g->player.pos.x, g->player.pos.y, 0, 0);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.state, PS_DEAD);
  game_free(g);
  free(g);
}

static void test_pod_docking_progression(void) {
  Game* g = new_playing_game(1);
  for (int n = 1; n <= NUM_COMPONENTS; n++) {
    sim_pickup(g, n, g->player.pos.x, g->player.pos.y - 2);
    tick_n(g, (Input){0}, 1);
    CHECK(g->player.comp[n - 1]);
    CHECK_INT(player_component_count(g), n);
    CHECK(!player_comp_docked(g, n - 1)); /* still sliding into its slot */
    tick_n(g, (Input){0}, DOCK_TICKS + 1);
    CHECK(player_comp_docked(g, n - 1));
  }
  CHECK_INT(g->stats_docks, NUM_COMPONENTS);
  game_free(g);
  free(g);
}

static void test_docking_animation_ends_in_slot(void) {
  Game* g = new_playing_game(1);
  sim_pickup(g, 1, g->player.pos.x - 30, g->player.pos.y - 20);
  g->player.pos.x = g->player.pos.x; /* pickup radius check: move onto it */
  ((Pickup*)first_active(&g->pickups))->pos = (Vec2){g->player.pos.x - 8, g->player.pos.y - 8};
  tick_n(g, (Input){0}, 1);
  Vec2 o0 = player_comp_offset(g, 0);
  tick_n(g, (Input){0}, DOCK_TICKS + 2);
  Vec2 o1 = player_comp_offset(g, 0);
  Vec2 slot = player_slot_offset(0);
  CHECK_NEAR(o1.x, slot.x, 0.001);
  CHECK_NEAR(o1.y, slot.y, 0.001);
  CHECK(fabsf(o0.x - slot.x) + fabsf(o0.y - slot.y) > 1.0f);
  game_free(g);
  free(g);
}

static void test_duplicate_pod_gives_bonus_not_second_component(void) {
  Game* g = new_playing_game(1);
  give_components(g, 1);
  long s = g->player.score;
  sim_pickup(g, 1, g->player.pos.x, g->player.pos.y);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(player_component_count(g), 1);
  CHECK(g->player.score > s);
  game_free(g);
  free(g);
}

static void test_weapons_grow_with_components(void) {
  /* each pair of modules changes the weapon's shape (needle, twin bolts, trident, lances, plasma); the second of a pair adds escorts */
  static const int ns[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  static const int bullets[] = {1, 2, 4, 3, 5, 5, 7, 5, 7};
  static const int damage[] = {1, 4, 6, 7, 9, 13, 15, 16, 18};
  for (int k = 0; k < 9; k++) {
    Game* g = new_playing_game(1);
    give_components(g, ns[k]);
    tick_n(g, (Input){.fire = true}, 1);
    int dmg = 0;
    POOL_EACH(PBullet, pb, g->pbullets) dmg += pb->dmg;
    CHECK_INT(count_active_pbullets(g), bullets[k]);
    CHECK_INT(dmg, damage[k]);
    game_free(g);
  free(g);
  }
}

static void test_component_hit_loses_component_not_life(void) {
  Game* g = new_playing_game(1);
  give_components(g, 4);
  int lives = g->player.lives;
  Vec2 o = player_slot_offset(0);
  sim_ebullet(g, EB_ORB, g->player.pos.x + o.x - 2, g->player.pos.y + o.y, 0, 0);
  tick_n(g, (Input){0}, 1);
  CHECK(!g->player.comp[0]);
  CHECK(g->player.comp[1] && g->player.comp[2] && g->player.comp[3]);
  CHECK_INT(g->player.lives, lives);
  CHECK_INT(g->player.state, PS_ALIVE);
  CHECK(g->player.invuln > 0);
  game_free(g);
  free(g);
}

static void test_death_loses_all_components(void) {
  Game* g = new_playing_game(1);
  give_components(g, 3);
  sim_ebullet(g, EB_ORB, g->player.pos.x, g->player.pos.y, 0, 0);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(player_component_count(g), 0);
  tick_n(g, (Input){0}, RESPAWN_DELAY + 2);
  CHECK_INT(player_component_count(g), 0);
  game_free(g);
  free(g);
}

static void test_c4_increases_speed(void) {
  Game* g = new_playing_game(1);
  float base = player_speed(g);
  give_components(g, 4);
  CHECK(player_speed(g) > base * 1.1f);
  game_free(g);
  free(g);
}

int main(void) {
  printf("test_player\n");
  RUN(test_eight_direction_movement);
  RUN(test_screen_bounds);
  RUN(test_shooting_cadence_and_simultaneous_move_fire);
  RUN(test_bullets_travel_up_and_expire);
  RUN(test_core_hit_kills_player_and_respawns_invulnerable);
  RUN(test_pod_docking_progression);
  RUN(test_docking_animation_ends_in_slot);
  RUN(test_duplicate_pod_gives_bonus_not_second_component);
  RUN(test_weapons_grow_with_components);
  RUN(test_component_hit_loses_component_not_life);
  RUN(test_death_loses_all_components);
  RUN(test_c4_increases_speed);
  return test_report();
}
