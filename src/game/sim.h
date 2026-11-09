/** Internal API shared by the simulation modules (player, enemy, ground, boss, level, game). */
#ifndef GAME_SIM_H_
#define GAME_SIM_H_

#include <math.h>

#include "animate.h"
#include "collision.h"
#include "game/game.h"

/** Centre + half-extent overlap, built on the shared engine routine aabb_collision(). */
static inline bool sim_overlap(float ax, float ay, float ahw, float ahh, float bx, float by, float bhw, float bhh) {
  return aabb_collision(ax - ahw, ay - ahh, ahw * 2, ahh * 2, bx - bhw, by - bhh, bhw * 2, bhh * 2);
}

/* ---- entity pools: the engine's object_pool, with the idiom used throughout ---- */
/** Walks the active objects of `pool` (a Game member): POOL_EACH(Enemy, e, g->enemies) { ... }. Releasing `e` inside is safe. */
#define POOL_EACH(T, var, pool)                                  \
  for (size_t var##_cursor = 0, var##_once = 1; var##_once; var##_once = 0) \
    for (T* var; (var = (T*)pool_next_active(&(pool), &var##_cursor)) != NULL;)
#define POOL_NEW(T, pool) ((T*)pool_acquire(&(pool), NULL))
#define POOL_FREE(pool, obj) pool_release_object(&(pool), (obj))
#define POOL_ALIVE(pool, obj) pool_is_active(&(pool), pool_index_of(&(pool), (obj)))
#define POOL_COUNT(pool) ((int)pool_get_active_count(&(pool)))

/* ---- motion: positions and velocities are the engine's point_t / velocity_t, advanced by animate() ---- */
#define SIM_DT (1.0 / TICKS_PER_SEC)
/** Sets a velocity from per-tick components (pixels per tick). */
static inline void sim_set_velocity(velocity_t* v, float vx, float vy) {
  float s = sqrtf(vx * vx + vy * vy);
  v->speed = (double)s * TICKS_PER_SEC;
  v->direction = s > 0.0f ? vector((double)(vx / s), (double)(vy / s)) : vector(0, 0);
}
static inline float sim_vx(const velocity_t* v) { return (float)(v->speed * v->direction.x * SIM_DT); }
static inline float sim_vy(const velocity_t* v) { return (float)(v->speed * v->direction.y * SIM_DT); }
/** One tick of movement. */
static inline void sim_move(point_t* p, velocity_t* v) { animate(p, v, SIM_DT); }

/* ---- helpers (game.c) ---- */
void sim_sfx(Game* g, Sfx s);
void sim_music(Game* g, Music m);
void sim_add_score(Game* g, long points);
/** Points for a kill after the fire-discipline bonus: x2.0 for a player who hardly fires down to x1.0 for one who never lets go. */
int sim_fire_bonus_tenths(const Game* g); /* 10..20 */
int sim_kill_points(const Game* g, int base);
Fx* sim_fx(Game* g, FxType type, float x, float y, bool world);
void sim_fx_score(Game* g, float x, float y, int value);
void sim_charge_gauge(Game* g, int amount);
float sim_ground_sy(const Game* g, float world_y);          /* screen y of a world y */
float sim_ground_wy(const Game* g, float screen_y);         /* world y of a screen y */
EBullet* sim_ebullet(Game* g, EBulletKind kind, float x, float y, float vx, float vy);
/** Fire an enemy orb from (x,y) toward the player at `speed`, with `angle_offset` radians of spread. */
void sim_aimed_shot(Game* g, float x, float y, float speed, float angle_offset);
/** Speed/rate scaling with difficulty. */
float sim_bullet_speed(const Game* g, float base);
int sim_fire_interval(const Game* g, int base);
Pickup* sim_pickup(Game* g, int number, float x, float y);
/** Drops a weapon capsule of a random type other than the one the player holds. */
void sim_weapon_drop(Game* g, float x, float y);
int player_weapon_interval(const Game* g);
bool ground_is_dino(GroundType t);
int sim_next_missing_component(const Game* g);
void sim_kill_all_enemy_bullets_to_fx(Game* g);

/* ---- player.c ---- */
void player_reset(Game* g, bool full);
void player_update(Game* g);
void player_hurtbox_core(const Game* g, float* x, float* y, float* hw, float* hh);
bool player_component_rect(const Game* g, int idx, float* x, float* y, float* hw, float* hh);
Vec2 player_slot_offset(int idx);
/** Offset of component `idx` (0..3) from the core for formation state/time. `rank`/`count` identify its place among attached ones. */
Vec2 formation_offset(int idx, int rank, int count, int state, int t);
bool player_can_start_formation(const Game* g);
void player_start_formation(Game* g);
int player_component_count(const Game* g);
/** Damage the player at a point/radius; returns true when something was hit. */
bool player_try_hit(Game* g, float x, float y, float radius);
bool player_hit_rect(Game* g, float x, float y, float hw, float hh);
void player_lose_component(Game* g, int idx);
void player_die(Game* g);
void player_dock(Game* g, int number, Vec2 from);
bool player_invulnerable(const Game* g);
bool player_is_phoenix(const Game* g);
float player_speed(const Game* g);

/* ---- enemy.c ---- */
void enemy_update_all(Game* g);
Enemy* enemy_spawn(Game* g, EnemyType type, float x, float y, float p0, int dir);
void enemy_hit(Game* g, Enemy* e, int dmg);
void enemy_half_size(EnemyType t, float* hw, float* hh);
int enemy_hp_for(const Game* g, EnemyType t);
int enemy_score(EnemyType t);

/* ---- ground.c ---- */
void ground_update_all(Game* g);
Ground* ground_spawn(Game* g, GroundType type, float wx, float wy, float p0);
void ground_hit(Game* g, Ground* e, int dmg);
void ground_half_size(GroundType t, float* hw, float* hh);
int ground_score(GroundType t);

/* ---- boss.c ---- */
void boss_start(Game* g);
void boss_update(Game* g);
bool boss_hit_test(Game* g, PBullet* b); /* returns true if the bullet was consumed */
void boss_core_pos(const Boss* b, float* x, float* y);
void boss_gun_pos(const Boss* b, int i, float* x, float* y);
bool boss_hurts_rect(const Game* g, float x, float y, float hw, float hh); /* beams */

/* ---- level.c ---- */
void level_build(Game* g, int stage);          /* spawn script + terrain map for the stage */
void level_update(Game* g);                    /* spawns due entities, advances scroll */
int level_theme(int stage);
float level_length(int stage);
uint16_t level_tile(const Game* g, int row, int col); /* row 0 = start of stage */
#define TILES_PER_THEME 33
int level_tile_sprite_local(uint16_t tile);

/* ---- demo ai ---- */
void demo_ai(const Game* g, Input* out);


/* extra player queries used by the renderer and tests */
Vec2 player_comp_offset(const Game* g, int idx); /* current offset of component idx from the core */
bool player_comp_docked(const Game* g, int idx); /* attached and fully docked */
int player_rank_of(const Game* g, int idx);      /* index among attached components */
#endif
