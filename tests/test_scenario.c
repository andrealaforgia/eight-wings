/* End-to-end deterministic scenarios: the demo pilot plays real stages through the real simulation. */
#include "helpers.h"

typedef struct {
  int max_components, docks, formations, bosses, kills;
  int stage_reached, loop;
  long score;
  uint32_t hash;
  int ticks;
  int mode;
} Result;

static uint32_t mix(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }

static Result play(uint32_t seed, int ticks, bool god, int start_stage) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, seed);
  g->god_mode = god;
  game_new_run(g, start_stage);
  Result r = {0};
  uint32_t h = 2166136261u;
  for (int t = 0; t < ticks && g->mode == MODE_PLAYING; t++) {
    Input in;
    demo_ai(g, &in);
    game_tick(g, &in);
    int c = player_component_count(g);
    if (c > r.max_components) r.max_components = c;
    h = mix(h, (uint32_t)(g->player.pos.x * 8) ^ ((uint32_t)(g->player.pos.y * 8) << 12));
    h = mix(h, (uint32_t)g->kills);
    r.ticks = t;
  }
  r.docks = g->stats_docks;
  r.formations = g->stats_formations;
  r.bosses = g->stats_bosses;
  r.kills = g->kills;
  r.stage_reached = g->stage;
  r.loop = g->loop;
  r.score = g->player.score;
  r.mode = g->mode;
  r.hash = mix(h, (uint32_t)g->player.score);
  game_free(g);
  free(g);
  return r;
}

static void test_pilot_exercises_all_major_mechanics(void) {
  Result r = play(1985, 60 * 60 * 6, true, 0);
  printf("    kills=%d docks=%d maxcomp=%d formations=%d bosses=%d stage=%d loop=%d score=%ld ticks=%d\n", r.kills, r.docks, r.max_components, r.formations,
         r.bosses, r.stage_reached, r.loop, r.score, r.ticks);
  CHECK(r.kills > 100);
  CHECK(r.docks >= 3);            /* pods collected, components docked */
  CHECK(r.max_components >= 6);   /* ship visibly grew */
  CHECK(r.formations >= 1);       /* formation attack used */
  CHECK(r.bosses >= 1);           /* boss defeated */
  CHECK(r.score > 10000);
}

static void test_runs_are_reproducible(void) {
  Result a = play(77, 60 * 60 * 2, false, 0), b = play(77, 60 * 60 * 2, false, 0), c = play(78, 60 * 60 * 2, false, 0);
  CHECK(a.hash == b.hash);
  CHECK_INT(a.score, b.score);
  CHECK(a.hash != c.hash);
}

static void test_unassisted_pilot_eventually_loses_and_game_over_works(void) {
  Result r = play(5, 60 * 60 * 40, false, 2); /* hardest stage, no god mode */
  CHECK_INT(r.mode, MODE_GAMEOVER);
}

static void test_every_stage_boss_can_be_beaten(void) {
  for (int st = 0; st < NUM_STAGES; st++) {
    Result r = play(123, 60 * 60 * 5, true, st);
    printf("    stage %d: bosses=%d kills=%d ticks=%d\n", st, r.bosses, r.kills, r.ticks);
    CHECK(r.bosses >= 1);
  }
}

int main(void) {
  printf("test_scenario\n");
  RUN(test_pilot_exercises_all_major_mechanics);
  RUN(test_runs_are_reproducible);
  RUN(test_unassisted_pilot_eventually_loses_and_game_over_works);
  RUN(test_every_stage_boss_can_be_beaten);
  return test_report();
}
