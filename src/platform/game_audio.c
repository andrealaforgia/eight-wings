#include "platform/game_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"

typedef struct {
  int16_t* data;
  int len; /* mono samples at MIXER_RATE */
} MusicClip;

struct Audio {
  audio_context_ptr engine;
  MusicClip mus[MUS_COUNT];
  volatile int music_id; /* read by the engine's audio callback */
  volatile int music_pos;
  float sfx_gain;
};

static const char* const kSfxNames[SFX_COUNT] = {
    [SFX_SHOT] = "shot", [SFX_SHOT_HEAVY] = "shot_heavy", [SFX_HIT] = "hit", [SFX_ENEMY_DIE] = "enemy_die", [SFX_EXPLODE_BIG] = "explode_big",
    [SFX_GROUND_DIE] = "ground_die", [SFX_POD_APPEAR] = "pod_appear", [SFX_DOCK] = "dock", [SFX_COMPONENT_LOST] = "component_lost",
    [SFX_FORMATION_START] = "formation_start", [SFX_FORMATION_END] = "formation_end", [SFX_PLAYER_DIE] = "player_die", [SFX_MENU_MOVE] = "menu_move",
    [SFX_MENU_SELECT] = "menu_select", [SFX_BOSS_WARNING] = "boss_warning", [SFX_BOSS_HIT] = "boss_hit", [SFX_BOSS_DIE] = "boss_die",
    [SFX_ONE_UP] = "one_up", [SFX_MEDAL] = "medal", [SFX_ENEMY_SHOT] = "enemy_shot", [SFX_BEAM] = "beam", [SFX_STAGE_CLEAR] = "stage_clear",
    [SFX_PAUSE] = "pause", [SFX_DINO_DIE] = "dino_die", [SFX_DINO_SPIT] = "dino_spit"};
static const char* const kMusNames[MUS_COUNT] = {[MUS_TITLE] = "title", [MUS_STAGE1] = "stage1", [MUS_STAGE2] = "stage2", [MUS_STAGE3] = "stage3",
                                                 [MUS_BOSS] = "boss", [MUS_GAMEOVER] = "gameover", [MUS_CLEAR] = "clear"};

/* Music loops are our own generated mono 16-bit WAVs (a 44-byte header, then samples). */
static bool load_music(MusicClip* c, const char* path) {
  FILE* f = fopen(path, "rb");
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 44, SEEK_SET);
  long bytes = size - 44;
  if (bytes <= 0) { fclose(f); return false; }
  c->data = (int16_t*)malloc((size_t)bytes);
  bool ok = c->data && fread(c->data, 1, (size_t)bytes, f) == (size_t)bytes;
  fclose(f);
  c->len = (int)(bytes / 2);
  if (!ok) { free(c->data); c->data = NULL; }
  return ok;
}

/* The engine calls this from its mixer to fetch `frames` samples of music. */
static void music_source(void* user, int16_t* out, int frames, float gain) {
  Audio* a = (Audio*)user;
  int id = a->music_id;
  const MusicClip* m = (id > MUS_NONE && id < MUS_COUNT) ? &a->mus[id] : NULL;
  int pos = a->music_pos;
  for (int i = 0; i < frames; i++) {
    if (!m || !m->data) { out[i] = 0; continue; }
    if (pos >= m->len) pos = 0;
    out[i] = (int16_t)((float)m->data[pos++] * gain);
  }
  a->music_pos = pos;
}

Audio* audio_create(const char* dir, int volume) {
  Audio* a = (Audio*)calloc(1, sizeof *a);
  if (!a) return NULL;
  a->engine = init_audio_context(SFX_COUNT, volume);
  if (!a->engine) {
    fprintf(stderr, "audio unavailable; continuing silently\n");
    free(a);
    return NULL;
  }
  char base[1100];
  snprintf(base, sizeof base, "%s/audio", dir);
  int loaded = 0;
  for (int i = 1; i < SFX_COUNT; i++) {
    char file[96];
    snprintf(file, sizeof file, "sfx_%s.wav", kSfxNames[i]);
    if (load_sound(a->engine, i, base, file)) loaded++;
  }
  for (int i = 1; i < MUS_COUNT; i++) {
    char path[1200];
    snprintf(path, sizeof path, "%s/mus_%s.wav", base, kMusNames[i]);
    if (load_music(&a->mus[i], path)) loaded++;
  }
  if (loaded == 0) {
    fprintf(stderr, "no wav files, run `make audio`; continuing silently\n");
    audio_destroy(a);
    return NULL;
  }
  a->sfx_gain = 0.8f;
  set_music_source(a->engine, music_source, a);
  set_music_gain(a->engine, 0.5f);
  return a;
}

void audio_destroy(Audio* a) {
  if (!a) return;
  terminate_audio_context(a->engine);
  for (int i = 0; i < MUS_COUNT; i++) free(a->mus[i].data);
  free(a);
}

void audio_play(Audio* a, Sfx s) {
  if (!a || s <= SFX_NONE || s >= SFX_COUNT) return;
  float v = (s == SFX_SHOT || s == SFX_ENEMY_SHOT || s == SFX_HIT) ? 0.55f : 1.0f;
  play_sound_at_volume(a->engine, (int)s, v * a->sfx_gain);
}

void audio_music(Audio* a, Music m) {
  if (!a || (int)m == a->music_id) return;
  a->music_pos = 0;
  a->music_id = (int)m;
}

void audio_set_volume(Audio* a, int music_vol, int sfx_vol, bool muted) {
  if (!a) return;
  set_music_enabled(a->engine, !muted);
  set_sound_enabled(a->engine, !muted);
  set_music_gain(a->engine, (float)music_vol / 10.0f * 0.6f);
  a->sfx_gain = (float)sfx_vol / 10.0f * 0.9f;
}
