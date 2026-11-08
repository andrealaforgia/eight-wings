/**
 * The whole deterministic simulation state. One Game, advanced by game_tick() at a fixed 60 Hz.
 * Fixed-size arrays only: nothing is allocated in the main loop.
 */
#ifndef GAME_GAME_H_
#define GAME_GAME_H_

#include "rng.h"
#include "game/types.h"

typedef enum { MODE_TITLE, MODE_CONTROLS, MODE_PLAYING, MODE_GAMEOVER, MODE_DEMO } Mode;
typedef enum { PH_INTRO, PH_PLAY, PH_BOSS, PH_CLEAR } StagePhase;
typedef enum { PS_ALIVE, PS_DEAD } PlayerState;
typedef enum { FORM_NONE, FORM_SCATTER, FORM_HOLD, FORM_REUNITE } FormState;

/* --- balance constants (explicit so they can be tested and tuned in one place) --- */
#define PLAYER_SPEED 1.7f
#define PLAYER_SPEED_C4_BONUS 1.18f
#define PLAYER_START_LIVES 3
#define PLAYER_SPAWN_X 128.0f
#define PLAYER_SPAWN_Y 190.0f
#define PLAYER_MARGIN_X 10.0f
#define PLAYER_TOP 16.0f
#define PLAYER_BOTTOM 212.0f
#define RESPAWN_DELAY 100
#define RESPAWN_INVULN 180
#define COMP_HIT_INVULN 75
#define SHOT_INTERVAL 7
#define DOCK_TICKS 26
#define GAUGE_MAX 100
#define FORMATION_MIN_COMPONENTS 2
#define FORM_SCATTER_TICKS 30
#define FORM_HOLD_TICKS 300
#define FORM_REUNITE_TICKS 32
#define FORM_COOLDOWN_TICKS 90
#define FORM_ORBIT_RADIUS 36.0f
#define FORM_ORBIT_RADIUS_INNER 30.0f
#define FORM_ORBIT_RADIUS_OUTER 48.0f
#define FORM_ORBIT_SPEED 0.075f /* radians per tick */
#define STAGE_INTRO_TICKS 150
#define STAGE_CLEAR_TICKS 260
#define GAMEOVER_TICKS 300
#define TITLE_TO_CONTROLS_TICKS 540
#define CONTROLS_TO_DEMO_TICKS 480
#define DEMO_TICKS 1800
#define EXTRA_LIFE_FIRST 30000
#define EXTRA_LIFE_EVERY 70000

typedef struct {
  Vec2 pos;
  int lives;
  long score;
  PlayerState state;
  int invuln;          /* ticks of invulnerability left */
  int respawn_timer;
  int shot_cd;
  int shot_alt;        /* alternates heavy shots */
  int bank;            /* -1 left, 0 straight, +1 right */
  bool comp[NUM_COMPONENTS];     /* attached (docked or docking) */
  int dock_t[NUM_COMPONENTS];    /* ticks into docking; >= DOCK_TICKS means fully docked */
  Vec2 dock_from[NUM_COMPONENTS];/* offset relative to the core where docking started */
  int form_state;
  int form_t;
  int form_cd;
  int gauge;           /* 0..GAUGE_MAX, charged by kills */
  int comp_shot_cd[NUM_COMPONENTS];
  int next_life_at;
  int hit_flash;
} Player;

typedef struct {
  bool active;
  int id;  /* 0..2 */
  Vec2 pos;
  int t;
  int phase;       /* 0 guns up, 1 core exposed, 2 enraged */
  int gun_hp[2];
  int gun_max;
  int core_hp, core_max;
  int flash_gun[2], flash_core, flash_body;
  int dying;       /* >0 while the death sequence runs */
  float spiral;
  Beam beams[MAX_BEAMS];
  int entering;    /* ticks of the entrance left */
} Boss;

typedef struct {
  int music_vol;   /* 0..10 */
  int sfx_vol;     /* 0..10 */
  bool muted;
} Settings;

typedef struct {
  /* meta */
  Mode mode;
  uint32_t seed;
  rng_t rng;
  long tick;           /* total ticks since game_init */
  long mode_tick;      /* ticks in the current mode */
  bool paused;
  int menu_cursor;
  Settings settings;
  long hiscore;
  Input in, prev_in;
  bool god_mode;       /* debug: player cannot be hurt */
  bool debug_overlay;

  /* run */
  Player player;
  int stage;           /* 0..NUM_STAGES-1 within the loop */
  int loop;            /* completed loops through all stages */
  int difficulty;      /* stage + NUM_STAGES * loop */
  StagePhase phase;
  long phase_tick;
  float scroll;        /* pixels travelled in this stage */
  float scroll_speed;
  float stage_length;
  int kills;
  int stats_docks, stats_formations, stats_bosses;

  /* world */
  PBullet pbullets[MAX_PBULLETS];
  EBullet ebullets[MAX_EBULLETS];
  Enemy enemies[MAX_ENEMIES];
  Ground ground[MAX_GROUND];
  Pickup pickups[MAX_PICKUPS];
  Fx fx[MAX_FX];
  Boss boss;

  /* level data */
  Spawn spawns[MAX_SPAWNS];
  int spawn_count;
  int spawn_next;
  uint16_t map[MAP_MAX_ROWS][MAP_COLS];
  int map_rows;

  /* output channels to the platform layer */
  Sfx sfx_queue[MAX_SFX_QUEUE];
  int sfx_count;
  Music music_request;
  float screen_flash;  /* 0..1 white flash */
  int shake;
} Game;

/* ---- lifecycle ---- */
void game_init(Game* g, uint32_t seed);
void game_new_run(Game* g, int start_stage);  /* begins a new game at the given stage */
void game_tick(Game* g, const Input* in);

#endif

/* FX animation table shared with the renderer. */
int fx_frame_count(FxType t);
int fx_ticks_per_frame(FxType t);
/** True the tick the key went down (edge), for menu handling. */
bool game_pressed_start(const Game* g);
