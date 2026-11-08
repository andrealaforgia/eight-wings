#include "helpers.h"

static void test_level_is_deterministic(void) {
  Game* a = (Game*)calloc(1, sizeof(Game));
  Game* b = (Game*)calloc(1, sizeof(Game));
  game_init(a, 42); game_init(b, 42);
  level_build(a, 1); level_build(b, 1);
  CHECK_INT(a->spawn_count, b->spawn_count);
  CHECK(memcmp(a->spawns, b->spawns, sizeof(Spawn) * (size_t)a->spawn_count) == 0);
  CHECK(memcmp(a->map, b->map, sizeof a->map) == 0);
  game_init(b, 43);
  level_build(b, 1);
  CHECK(memcmp(a->map, b->map, sizeof a->map) != 0);
  free(a); free(b);
}

static void test_level_script_sorted_and_contains_all_kinds(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1985);
  for (int st = 0; st < NUM_STAGES; st++) {
    level_build(g, st);
    int air[ET_COUNT] = {0}, gr[GT_COUNT] = {0};
    for (int i = 0; i < g->spawn_count; i++) {
      if (i) CHECK(g->spawns[i].at >= g->spawns[i - 1].at);
      CHECK(g->spawns[i].at < level_length(st) + 200);
      if (g->spawns[i].kind == SP_AIR) air[g->spawns[i].type]++; else gr[g->spawns[i].type]++;
    }
    CHECK(g->spawn_count < MAX_SPAWNS);
    CHECK(gr[GT_BUNKER] >= 5);  /* enough component supply */
    CHECK(gr[GT_TURRET] > 0 && gr[GT_FORT] > 0);
    CHECK(gr[GT_RAPTOR] >= 3 && gr[GT_REX] >= 1); /* dinosaurs in every stage */
    CHECK(air[ET_DRONE] > 0 && air[ET_SWOOP] > 0 && air[ET_WING] > 0);
    if (st >= 1) CHECK(air[ET_BOMBER] > 0 && air[ET_SAUCER] > 0 && air[ET_SPINNER] > 0 && air[ET_DIVER] > 0);
  }
  game_free(g);
  free(g);
}

static void test_difficulty_escalates_across_stages(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1985);
  int count[NUM_STAGES];
  for (int st = 0; st < NUM_STAGES; st++) { level_build(g, st); count[st] = g->spawn_count; }
  CHECK(count[2] > count[0]);
  CHECK(level_length(2) > level_length(0));
  game_free(g);
  free(g);
}

static void test_terrain_ground_targets_stand_on_land(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1985);
  for (int st = 0; st < NUM_STAGES; st++) {
    level_build(g, st);
    for (int i = 0; i < g->spawn_count; i++) {
      if (g->spawns[i].kind != SP_GROUND) continue;
      int col = (int)(g->spawns[i].x / TILE), row = (int)(g->spawns[i].p0 / TILE);
      int local = level_tile_sprite_local(level_tile(g, row, col));
      CHECK(local < 17 || local >= 17 + 16 ? 1 : 0);  /* not a liquid tile */
      CHECK(local < 17);
    }
  }
  game_free(g);
  free(g);
}

static void test_liquid_autotile_masks_match_neighbours(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1985);
  level_build(g, 1);
  int liquids = 0;
  for (int r = 1; r < g->map_rows - 1; r++)
    for (int c = 1; c < MAP_COLS - 1; c++) {
      int l = level_tile_sprite_local(level_tile(g, r, c));
      if (l < 17 || l > 32) continue;
      liquids++;
      int mask = l - 17;
      int nl[4] = {level_tile_sprite_local(level_tile(g, r + 1, c)), level_tile_sprite_local(level_tile(g, r, c + 1)),
                   level_tile_sprite_local(level_tile(g, r - 1, c)), level_tile_sprite_local(level_tile(g, r, c - 1))};
      for (int k = 0; k < 4; k++) {
        bool neigh_liquid = nl[k] >= 17 && nl[k] <= 32;
        bool bit = (mask >> k) & 1;
        if (!bit) CHECK(neigh_liquid);          /* no shore bit => neighbour is liquid */
        else CHECK(!neigh_liquid);              /* shore bit => neighbour is land (or road) */
      }
    }
  CHECK(liquids > 100);  /* the archipelago stage really has water */
  game_free(g);
  free(g);
}

static void test_scrolling_and_spawning(void) {
  Game* g = new_playing_game(1);
  g->spawns[0] = (Spawn){10.0f, SP_AIR, ET_DRONE, 100, 0, 1, 0};
  g->spawn_count = 1;
  g->scroll = 0;
  tick_n(g, (Input){0}, 5);
  int active = POOL_COUNT(g->enemies);
  CHECK_INT(active, 0);
  tick_n(g, (Input){0}, 20);
  active = POOL_COUNT(g->enemies);
  CHECK_INT(active, 1);
  game_free(g);
  free(g);
}

/* ---- game flow ---- */

static void test_title_to_play_and_restart(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1);
  CHECK_INT(g->mode, MODE_TITLE);
  tick_n(g, (Input){0}, 5);
  CHECK_INT(g->mode, MODE_TITLE);
  tick_n(g, (Input){.start = true}, 1);
  CHECK_INT(g->mode, MODE_PLAYING);
  CHECK_INT(g->player.lives, PLAYER_START_LIVES);
  /* die three times -> game over */
  for (int i = 0; i < PLAYER_START_LIVES; i++) {
    g->player.invuln = 0;
    g->player.state = PS_ALIVE;
    player_die(g);
    tick_n(g, (Input){0}, RESPAWN_DELAY + 5);
  }
  CHECK_INT(g->mode, MODE_GAMEOVER);
  tick_n(g, (Input){0}, 100);
  tick_n(g, (Input){.start = true}, 1);
  CHECK_INT(g->mode, MODE_PLAYING);
  CHECK_INT(g->player.lives, PLAYER_START_LIVES);
  CHECK_INT(g->player.score, 0);
  game_free(g);
  free(g);
}

static void test_pause_and_menu(void) {
  Game* g = new_playing_game(1);
  float x = g->player.pos.x;
  tick_n(g, (Input){.pause = true}, 1);
  tick_n(g, (Input){0}, 1);
  CHECK(g->paused);
  tick_n(g, (Input){.right = true}, 1); /* menu is focused on RESUME: movement frozen */
  tick_n(g, (Input){0}, 1);
  CHECK_NEAR(g->player.pos.x, x, 0.001);
  /* go to MUSIC volume and adjust */
  int v = g->settings.music_vol;
  tick_n(g, (Input){.down = true}, 1); tick_n(g, (Input){0}, 1);
  tick_n(g, (Input){.left = true}, 1); tick_n(g, (Input){0}, 1);
  CHECK_INT(g->settings.music_vol, v - 1);
  /* mute toggle */
  tick_n(g, (Input){.down = true}, 1); tick_n(g, (Input){0}, 1);
  tick_n(g, (Input){.down = true}, 1); tick_n(g, (Input){0}, 1);
  tick_n(g, (Input){.start = true}, 1); tick_n(g, (Input){0}, 1);
  CHECK(g->settings.muted);
  /* escape resumes */
  tick_n(g, (Input){.pause = true}, 1); tick_n(g, (Input){0}, 1);
  CHECK(!g->paused);
  game_free(g);
  free(g);
}

static void test_scoring_extra_life_and_hiscore(void) {
  Game* g = new_playing_game(1);
  int lives = g->player.lives;
  sim_add_score(g, EXTRA_LIFE_FIRST - 1);
  CHECK_INT(g->player.lives, lives);
  sim_add_score(g, 1);
  CHECK_INT(g->player.lives, lives + 1);
  sim_add_score(g, EXTRA_LIFE_EVERY - 1);
  CHECK_INT(g->player.lives, lives + 1);
  sim_add_score(g, 1);
  CHECK_INT(g->player.lives, lives + 2);
  CHECK_INT(g->hiscore, g->player.score);
  game_free(g);
  free(g);
}

static void test_hiscore_survives_new_run(void) {
  Game* g = new_playing_game(1);
  sim_add_score(g, 12345);
  game_new_run(g, 0);
  CHECK_INT(g->player.score, 0);
  CHECK_INT(g->hiscore, 12345);
  game_free(g);
  free(g);
}

static void test_attract_mode_cycle(void) {
  Game* g = (Game*)calloc(1, sizeof(Game));
  game_init(g, 1);
  tick_n(g, (Input){0}, TITLE_TO_CONTROLS_TICKS + 5);
  CHECK_INT(g->mode, MODE_CONTROLS);
  tick_n(g, (Input){0}, CONTROLS_TO_DEMO_TICKS + 5);
  CHECK_INT(g->mode, MODE_DEMO);
  tick_n(g, (Input){0}, 300);
  CHECK_INT(g->mode, MODE_DEMO);
  tick_n(g, (Input){.fire = true}, 1); /* any key leaves attract mode */
  CHECK_INT(g->mode, MODE_TITLE);
  game_free(g);
  free(g);
}

static void test_stage_loop_raises_difficulty(void) {
  Game* g = new_playing_game(1);
  for (int i = 0; i < NUM_STAGES; i++) {
    g->scroll = g->stage_length;
    tick_n(g, (Input){0}, 2);
    g->player.invuln = 99999;
    tick_n(g, (Input){0}, 140);
    g->boss.dying = 1;
    tick_n(g, (Input){0}, 200 + STAGE_CLEAR_TICKS + 5);
  }
  CHECK_INT(g->loop, 1);
  CHECK_INT(g->stage, 0);
  CHECK_INT(g->difficulty, NUM_STAGES);
  game_free(g);
  free(g);
}

int main(void) {
  printf("test_level_game\n");
  RUN(test_level_is_deterministic);
  RUN(test_level_script_sorted_and_contains_all_kinds);
  RUN(test_difficulty_escalates_across_stages);
  RUN(test_terrain_ground_targets_stand_on_land);
  RUN(test_liquid_autotile_masks_match_neighbours);
  RUN(test_scrolling_and_spawning);
  RUN(test_title_to_play_and_restart);
  RUN(test_pause_and_menu);
  RUN(test_scoring_extra_life_and_hiscore);
  RUN(test_hiscore_survives_new_run);
  RUN(test_attract_mode_cycle);
  RUN(test_stage_loop_raises_difficulty);
  return test_report();
}
