#include "helpers.h"

static float len(Vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }

static void test_geometry_start_orbit_end(void) {
  for (int count = 2; count <= 4; count++) {
    for (int rank = 0; rank < count; rank++) {
      int idx = rank; /* components docked in order */
      Vec2 slot = player_slot_offset(idx);
      Vec2 a = formation_offset(idx, rank, count, FORM_SCATTER, 0);
      CHECK_NEAR(a.x, slot.x, 0.001);
      CHECK_NEAR(a.y, slot.y, 0.001);
      for (int t = FORM_SCATTER_TICKS; t < FORM_SCATTER_TICKS + FORM_HOLD_TICKS; t += 17) {
        Vec2 o = formation_offset(idx, rank, count, FORM_HOLD, t);
        CHECK_NEAR(len(o), FORM_ORBIT_RADIUS, 0.01); /* constant orbit radius */
      }
      int end = FORM_SCATTER_TICKS + FORM_HOLD_TICKS + FORM_REUNITE_TICKS;
      Vec2 e = formation_offset(idx, rank, count, FORM_REUNITE, end);
      CHECK_NEAR(e.x, slot.x, 0.001); /* reunion lands exactly in the slot */
      CHECK_NEAR(e.y, slot.y, 0.001);
    }
  }
}

static void test_components_evenly_spaced_in_orbit(void) {
  int t = FORM_SCATTER_TICKS + 40;
  Vec2 a = formation_offset(0, 0, 4, FORM_HOLD, t), b = formation_offset(1, 1, 4, FORM_HOLD, t);
  Vec2 c = formation_offset(2, 2, 4, FORM_HOLD, t), d = formation_offset(3, 3, 4, FORM_HOLD, t);
  CHECK_NEAR(a.x + c.x, 0, 0.01); /* opposite pairs */
  CHECK_NEAR(a.y + c.y, 0, 0.01);
  CHECK_NEAR(b.x + d.x, 0, 0.01);
  CHECK_NEAR(b.y + d.y, 0, 0.01);
}

static void test_eight_modules_use_two_counter_rotating_rings(void) {
  for (int rank = 0; rank < 8; rank++) {
    Vec2 a = formation_offset(rank, rank, 8, FORM_HOLD, FORM_SCATTER_TICKS + 50);
    float want = rank % 2 == 0 ? FORM_ORBIT_RADIUS_INNER : FORM_ORBIT_RADIUS_OUTER;
    CHECK_NEAR(len(a), want, 0.01);
    Vec2 s = player_slot_offset(rank);
    Vec2 e = formation_offset(rank, rank, 8, FORM_REUNITE, FORM_SCATTER_TICKS + FORM_HOLD_TICKS + FORM_REUNITE_TICKS);
    CHECK_NEAR(e.x, s.x, 0.001);
    CHECK_NEAR(e.y, s.y, 0.001);
  }
  /* rings counter-rotate: after a quarter turn of inner motion the outer one went the other way */
  Vec2 i0 = formation_offset(0, 0, 8, FORM_HOLD, 40), i1 = formation_offset(0, 0, 8, FORM_HOLD, 60);
  Vec2 o0 = formation_offset(1, 1, 8, FORM_HOLD, 40), o1 = formation_offset(1, 1, 8, FORM_HOLD, 60);
  float cross_i = i0.x * i1.y - i0.y * i1.x, cross_o = o0.x * o1.y - o0.y * o1.x;
  CHECK(cross_i * cross_o < 0);
}

static void test_requirements_to_start(void) {
  Game* g = new_playing_game(3);
  g->player.gauge = GAUGE_MAX;
  CHECK(!player_can_start_formation(g)); /* no components */
  give_components(g, 1);
  CHECK(!player_can_start_formation(g)); /* one is not enough */
  give_components(g, 2);
  CHECK(player_can_start_formation(g));
  g->player.gauge = GAUGE_MAX - 1;
  CHECK(!player_can_start_formation(g)); /* gauge must be full */
  free(g);
}

static void test_full_attack_lifecycle(void) {
  Game* g = new_playing_game(3);
  give_components(g, 3);
  g->player.gauge = GAUGE_MAX;
  tick_n(g, (Input){.formation = true}, 1);
  CHECK_INT(g->player.form_state, FORM_SCATTER);
  CHECK_INT(g->stats_formations, 1);
  CHECK(player_invulnerable(g));
  tick_n(g, (Input){0}, FORM_SCATTER_TICKS);
  CHECK_INT(g->player.form_state, FORM_HOLD);
  /* in the air the components fire on their own */
  int shots = 0;
  for (int i = 0; i < 40; i++) { tick_n(g, (Input){0}, 1); shots += count_active_pbullets(g) > 0; }
  CHECK(shots > 0);
  /* invulnerable to bullets, which are absorbed */
  sim_ebullet(g, EB_ORB, g->player.pos.x, g->player.pos.y, 0, 0);
  tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.state, PS_ALIVE);
  for (int i = 0; i < FORM_HOLD_TICKS && g->player.form_state != FORM_REUNITE; i++) tick_n(g, (Input){0}, 1);
  CHECK_INT(g->player.form_state, FORM_REUNITE);
  tick_n(g, (Input){0}, FORM_REUNITE_TICKS + 2);
  CHECK_INT(g->player.form_state, FORM_NONE);
  CHECK_INT(player_component_count(g), 3); /* nobody lost */
  for (int i = 0; i < 3; i++) {
    Vec2 o = player_comp_offset(g, i), s = player_slot_offset(i);
    CHECK_NEAR(o.x, s.x, 0.01);
    CHECK_NEAR(o.y, s.y, 0.01);
  }
  /* cooldown + empty gauge: cannot immediately repeat */
  CHECK_INT(g->player.gauge, 0);
  g->player.gauge = GAUGE_MAX;
  CHECK(g->player.form_cd > 0 && !player_can_start_formation(g));
  tick_n(g, (Input){0}, FORM_COOLDOWN_TICKS + 1);
  CHECK(player_can_start_formation(g));
  free(g);
}

static void test_phoenix_only_with_full_assembly(void) {
  Game* g = new_playing_game(3);
  give_components(g, 3);
  g->player.gauge = GAUGE_MAX;
  tick_n(g, (Input){.formation = true}, 1);
  CHECK(!player_is_phoenix(g));
  free(g);
  g = new_playing_game(3);
  give_components(g, NUM_COMPONENTS - 1);
  g->player.gauge = GAUGE_MAX;
  tick_n(g, (Input){.formation = true}, 1);
  CHECK(!player_is_phoenix(g)); /* seven of eight is not enough */
  free(g);
  g = new_playing_game(3);
  give_components(g, NUM_COMPONENTS);
  g->player.gauge = GAUGE_MAX;
  tick_n(g, (Input){.formation = true}, 1);
  CHECK(player_is_phoenix(g));
  tick_n(g, (Input){.fire = true}, 30);
  bool phoenix_shot = false;
  for (int i = 0; i < MAX_PBULLETS; i++) if (g->pbullets[i].active && g->pbullets[i].kind == PB_PHOENIX) phoenix_shot = true;
  CHECK(phoenix_shot);
  free(g);
}

static void test_formation_rams_enemies(void) {
  Game* g = new_playing_game(3);
  give_components(g, 2);
  g->player.gauge = GAUGE_MAX;
  tick_n(g, (Input){.formation = true}, FORM_SCATTER_TICKS + 5);
  Vec2 o = player_comp_offset(g, 0);
  Enemy* e = enemy_spawn(g, ET_BOMBER, g->player.pos.x + o.x, g->player.pos.y + o.y, 0, 1);
  int hp = e->hp;
  tick_n(g, (Input){0}, 8);
  CHECK(!e->active || e->hp < hp);
  free(g);
}

static void test_gauge_charges_from_kills_and_not_while_active(void) {
  Game* g = new_playing_game(3);
  CHECK_INT(g->player.gauge, 0);
  Enemy* e = enemy_spawn(g, ET_DRONE, 100, 50, 0, 1);
  enemy_hit(g, e, 5);
  CHECK(g->player.gauge > 0);
  give_components(g, 2);
  g->player.gauge = GAUGE_MAX;
  tick_n(g, (Input){.formation = true}, 5);
  int gauge = g->player.gauge;
  sim_charge_gauge(g, 50);
  CHECK_INT(g->player.gauge, gauge);
  free(g);
}

int main(void) {
  printf("test_formation\n");
  RUN(test_geometry_start_orbit_end);
  RUN(test_components_evenly_spaced_in_orbit);
  RUN(test_eight_modules_use_two_counter_rotating_rings);
  RUN(test_requirements_to_start);
  RUN(test_full_attack_lifecycle);
  RUN(test_phoenix_only_with_full_assembly);
  RUN(test_formation_rams_enemies);
  RUN(test_gauge_charges_from_kills_and_not_while_active);
  return test_report();
}
