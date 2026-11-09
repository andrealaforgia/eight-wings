/** Shared constants and plain data types for the deterministic game simulation (no platform library here). */
#ifndef GAME_TYPES_H_
#define GAME_TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "geometry.h"
#include "physics.h"

#define SCREEN_W 256
#define SCREEN_H 224
#define TICKS_PER_SEC 60
#define TILE 16
#define MAP_COLS (SCREEN_W / TILE)

#define MAX_PBULLETS 180
#define MAX_EBULLETS 160
#define MAX_ENEMIES 48
#define MAX_GROUND 48
#define MAX_PICKUPS 8
#define MAX_FX 160
#define MAX_SPAWNS 700
#define MAX_BEAMS 4
#define NUM_COMPONENTS 8
#define NUM_STAGES 3
#define MAP_MAX_ROWS 1400

typedef struct {
  float x, y;
} Vec2;

/** Held-key state for one tick. Edge detection (press events) is done by the game. */
typedef struct {
  bool up, down, left, right;
  bool fire, formation;
  bool start;  /* Enter */
  bool pause;  /* Escape */
  bool menu_left, menu_right;  /* menu adjust (shares keys with left/right) */
} Input;

/* Sound effects requested by the simulation; the platform layer plays them. */
typedef enum {
  SFX_NONE,
  SFX_SHOT,
  SFX_SHOT_HEAVY,
  SFX_HIT,
  SFX_ENEMY_DIE,
  SFX_EXPLODE_BIG,
  SFX_GROUND_DIE,
  SFX_POD_APPEAR,
  SFX_DOCK,
  SFX_COMPONENT_LOST,
  SFX_FORMATION_START,
  SFX_FORMATION_END,
  SFX_PLAYER_DIE,
  SFX_MENU_MOVE,
  SFX_MENU_SELECT,
  SFX_BOSS_WARNING,
  SFX_BOSS_HIT,
  SFX_BOSS_DIE,
  SFX_ONE_UP,
  SFX_MEDAL,
  SFX_ENEMY_SHOT,
  SFX_BEAM,
  SFX_STAGE_CLEAR,
  SFX_PAUSE,
  SFX_DINO_DIE,
  SFX_DINO_SPIT,
  SFX_COUNT
} Sfx;

typedef enum { MUS_NONE, MUS_TITLE, MUS_STAGE1, MUS_STAGE2, MUS_STAGE3, MUS_BOSS, MUS_GAMEOVER, MUS_CLEAR, MUS_COUNT } Music;

/* ---- enemies ---- */
typedef enum { ET_DRONE, ET_SWOOP, ET_DIVER, ET_SPINNER, ET_WING, ET_BOMBER, ET_SAUCER, ET_WEAVER, ET_HUNTER, ET_GUNSHIP, ET_SPLITTER, ET_COUNT } EnemyType;

typedef struct {
  EnemyType type;
  point_t pos;       /* engine point and velocity: moved with the engine's animate() */
  velocity_t vel;
  int hp;
  int age;
  int timer;
  int state;
  float p0, p1;  /* per-type parameters (amplitude, phase, heading ...) */
  int dir;       /* +1 / -1 */
  int flash;     /* hit flash ticks */
} Enemy;

/* ---- ground targets (positions in WORLD coordinates: sy = SCREEN_H - wy + scroll) ---- */
typedef enum { GT_TURRET, GT_BUNKER, GT_TANK, GT_SILO, GT_FORT, GT_RADAR, GT_RAPTOR, GT_REX, GT_TRIKE, GT_SPINO, GT_BRONTO, GT_COUNT } GroundType;

typedef struct {
  GroundType type;
  Vec2 pos;  /* world x, world y (y grows upward with progress) */
  int hp;
  int age;
  int timer;
  int state;    /* bunker: 0 closed, 1 beacon; silo: hatch frame; */
  int aim;      /* turret frame 0..7 */
  int flash;
  float p0;   /* tank: direction; dinosaurs: heading (0 = walking down the screen, +x to the right) */
} Ground;

typedef struct {
  int number;  /* 1..8 module pod, 0 = medal, -1..-5 = weapon capsule (WeaponType + 1 negated) */
  Vec2 pos;
  int age;
} Pickup;

/* The ship's weapon type: picked up as hexagonal capsules, upgraded in power and shape by the number of docked modules. */
typedef enum { WT_VULCAN, WT_SPREAD, WT_LASER, WT_MISSILE, WT_WAVE, WT_COUNT } WeaponType;
typedef enum { PB_NORMAL, PB_WIDE, PB_FORM, PB_PHOENIX, PB_HEAVY, PB_BOLT, PB_LANCE, PB_PLASMA, PB_PELLET, PB_LASER, PB_MISSILE, PB_WAVE } PBulletKind;
typedef struct {
  PBulletKind kind;
  point_t pos;
  velocity_t vel;
  int dmg;
  bool pierce;
  int age;
  int life;    /* ticks left before it fades (0 = until it leaves the screen) */
  float ox;    /* waves: the column it oscillates around; missiles: unused */
  float phase; /* waves: oscillation phase */
  float amp;   /* waves: oscillation amplitude */
} PBullet;

typedef enum { EB_ORB, EB_MISSILE } EBulletKind;
typedef struct {
  EBulletKind kind;
  point_t pos;
  velocity_t vel;
  float accel;  /* missile acceleration along y */
  int age;
} EBullet;

typedef enum { FX_EXPL_S, FX_EXPL_L, FX_EXPL_G, FX_SPARK, FX_MUZZLE, FX_DEATH, FX_DOCK, FX_SCORE, FX_COUNT } FxType;
typedef struct {
  FxType type;
  Vec2 pos;    /* screen coords, or world coords when `world` */
  bool world;  /* scrolls with the ground */
  int t;
  int value;   /* FX_SCORE: points */
} Fx;

typedef struct {
  float x;
  int warn;  /* ticks of telegraph left */
  int on;    /* ticks of damage left */
} Beam;

/* ---- level script ---- */
typedef enum { SP_AIR, SP_GROUND } SpawnKind;
typedef struct {
  float at;  /* air: scroll distance that triggers it; ground: world y of the target */
  SpawnKind kind;
  int type;
  float x;
  float p0;
  int dir;
  int extra;
} Spawn;

#endif
