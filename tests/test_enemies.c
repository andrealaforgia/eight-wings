#include "helpers.h"

static void test_player_bullet_kills_drone_and_scores(void) {
  Game* g = new_playing_game(5);
  Enemy* e = enemy_spawn(g, ET_DRONE, 128, 100, 0, 1);
  long s = g->player.score;
  add_pbullet(g, PB_NORMAL, e->pos.x, e->pos.y, 0, -5, 1);
  tick_n(g, (Input){0}, 1);
  CHECK(!POOL_ALIVE(g->enemies, e));
  CHECK_INT(g->player.score - s, sim_kill_points(g, enemy_score(ET_DRONE)));
  CHECK_INT(g->kills, 1);
  game_free(g);
  free(g);
}

static void test_multi_hit_enemy(void) {
  Game* g = new_playing_game(5);
  Enemy* e = enemy_spawn(g, ET_SPINNER, 128, 100, 0, 1);
  int hp = e->hp;
  CHECK(hp >= 3);
  enemy_hit(g, e, 1);
  CHECK(POOL_ALIVE(g->enemies, e));
  CHECK_INT(e->hp, hp - 1);
  CHECK(e->flash > 0);
  enemy_hit(g, e, hp);
  CHECK(!POOL_ALIVE(g->enemies, e));
  game_free(g);
  free(g);
}

static void test_drone_sine_path(void) {
  Game* g = new_playing_game(5);
  Enemy* e = enemy_spawn(g, ET_DRONE, 128, -10, 0, 1);
  float minx = 999, maxx = -999;
  for (int i = 0; i < 150 && POOL_ALIVE(g->enemies, e); i++) {
    tick_n(g, (Input){0}, 1);
    if (e->pos.x < minx) minx = e->pos.x;
    if (e->pos.x > maxx) maxx = e->pos.x;
  }
  CHECK(maxx - minx > 40); /* weaves */
  CHECK(e->pos.y > 100);   /* and descends */
  game_free(g);
  free(g);
}

static void test_swoop_makes_a_u_turn_and_leaves(void) {
  Game* g = new_playing_game(5);
  g->player.pos = (Vec2){128, 200};
  Enemy* e = enemy_spawn(g, ET_SWOOP, -10, 10, 0, 1);
  float maxy = 0;
  for (int i = 0; i < 400 && POOL_ALIVE(g->enemies, e); i++) {
    tick_n(g, (Input){0}, 1);
    if (e->pos.y > maxy) maxy = e->pos.y;
    g->player.invuln = 999;
  }
  CHECK(maxy > 50 && maxy < 200);
  CHECK(!POOL_ALIVE(g->enemies, e)); /* left the battlefield */
  game_free(g);
  free(g);
}

static void test_diver_locks_on_and_dashes(void) {
  Game* g = new_playing_game(5);
  g->player.invuln = 99999;
  Enemy* e = enemy_spawn(g, ET_DIVER, 40, -10, 0, 1);
  tick_n(g, (Input){0}, 120);
  CHECK_INT(e->state, 2);
  CHECK(sim_vx(&e->vel) > 0.5f && sim_vy(&e->vel) > 0.5f); /* heading toward the player at bottom centre */
  game_free(g);
  free(g);
}

static void test_wing_formation_peels_off(void) {
  Game* g = new_playing_game(5);
  g->player.invuln = 99999;
  Enemy* e = enemy_spawn(g, ET_WING, 128, -10, 0, -1);
  tick_n(g, (Input){0}, 100);
  CHECK_INT(e->state, 1);
  CHECK(sim_vx(&e->vel) < 0);
  game_free(g);
  free(g);
}

static void test_enemy_shoots_aimed_at_player(void) {
  Game* g = new_playing_game(5);
  g->player.invuln = 99999;
  g->player.pos = (Vec2){200, 200};
  sim_aimed_shot(g, 50, 50, 2.0f, 0);
  EBullet* b = (EBullet*)first_active(&g->ebullets);
  CHECK(b != NULL);
  CHECK(sim_vx(&b->vel) > 0 && sim_vy(&b->vel) > 0);
  CHECK_NEAR(atan2f(sim_vx(&b->vel), sim_vy(&b->vel)), atan2f(150.0f, 150.0f), 0.01);
  game_free(g);
  free(g);
}

static void test_contact_with_enemy_kills_player(void) {
  Game* g = new_playing_game(5);
  enemy_spawn(g, ET_DRONE, g->player.pos.x, g->player.pos.y, 0, 1);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.state, PS_DEAD);
  game_free(g);
  free(g);
}

static void test_difficulty_scales_bullets(void) {
  Game* g = new_playing_game(5);
  g->difficulty = 0;
  float a = sim_bullet_speed(g, 1.0f);
  int ia = sim_fire_interval(g, 100);
  g->difficulty = 6;
  CHECK(sim_bullet_speed(g, 1.0f) > a);
  CHECK(sim_fire_interval(g, 100) < ia);
  g->difficulty = 3;
  Enemy* e = enemy_spawn(g, ET_BOMBER, 100, 0, 0, 1);
  int hp3 = e->hp;
  g->difficulty = 0;
  CHECK(enemy_hp_for(g, ET_BOMBER) < hp3);
  game_free(g);
  free(g);
}

static void test_bomber_drops_medal(void) {
  Game* g = new_playing_game(5);
  Enemy* e = enemy_spawn(g, ET_BOMBER, 100, 50, 0, 1);
  enemy_hit(g, e, 999);
  int medals = 0;
  POOL_EACH(Pickup, k, g->pickups) medals += (k->number == 0);
  CHECK_INT(medals, 1);
  game_free(g);
  free(g);
}

int main(void) {
  printf("test_enemies\n");
  RUN(test_player_bullet_kills_drone_and_scores);
  RUN(test_multi_hit_enemy);
  RUN(test_drone_sine_path);
  RUN(test_swoop_makes_a_u_turn_and_leaves);
  RUN(test_diver_locks_on_and_dashes);
  RUN(test_wing_formation_peels_off);
  RUN(test_enemy_shoots_aimed_at_player);
  RUN(test_contact_with_enemy_kills_player);
  RUN(test_difficulty_scales_bullets);
  RUN(test_bomber_drops_medal);
  return test_report();
}
