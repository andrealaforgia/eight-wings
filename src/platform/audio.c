#include "platform/audio.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOICES 16
#define AUDIO_RATE 22050

typedef struct {
  int16_t* data;
  uint32_t len; /* samples */
} Clip;

typedef struct {
  const Clip* clip;
  uint32_t pos;
  float gain;
} Voice;

struct Audio {
  SDL_AudioDeviceID dev;
  Clip sfx[SFX_COUNT];
  Clip mus[MUS_COUNT];
  Voice voices[VOICES];
  const Clip* music;
  Music music_id;
  uint32_t music_pos;
  float music_gain, sfx_gain;
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

static bool load_clip(Clip* c, const char* path) {
  SDL_AudioSpec spec;
  uint8_t* buf;
  uint32_t len;
  if (!SDL_LoadWAV(path, &spec, &buf, &len)) return false;
  if (spec.format != AUDIO_S16LSB || spec.channels != 1) { SDL_FreeWAV(buf); return false; }
  c->data = (int16_t*)malloc(len);
  memcpy(c->data, buf, len);
  c->len = len / 2;
  SDL_FreeWAV(buf);
  return true;
}

static void callback(void* user, uint8_t* stream, int bytes) {
  Audio* a = (Audio*)user;
  int16_t* out = (int16_t*)stream;
  int n = bytes / 2;
  for (int i = 0; i < n; i++) {
    float acc = 0;
    if (a->music && a->music_gain > 0) {
      acc += (float)a->music->data[a->music_pos] * a->music_gain;
      if (++a->music_pos >= a->music->len) a->music_pos = 0;
    }
    for (int v = 0; v < VOICES; v++) {
      Voice* vo = &a->voices[v];
      if (!vo->clip) continue;
      acc += (float)vo->clip->data[vo->pos] * vo->gain * a->sfx_gain;
      if (++vo->pos >= vo->clip->len) vo->clip = NULL;
    }
    if (acc > 32767) acc = 32767;
    if (acc < -32768) acc = -32768;
    out[i] = (int16_t)acc;
  }
}

Audio* audio_create(const char* dir) {
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return NULL;
  Audio* a = (Audio*)calloc(1, sizeof *a);
  char path[1024];
  int loaded = 0;
  for (int i = 1; i < SFX_COUNT; i++) {
    snprintf(path, sizeof path, "%s/audio/sfx_%s.wav", dir, kSfxNames[i]);
    if (load_clip(&a->sfx[i], path)) loaded++;
  }
  for (int i = 1; i < MUS_COUNT; i++) {
    snprintf(path, sizeof path, "%s/audio/mus_%s.wav", dir, kMusNames[i]);
    if (load_clip(&a->mus[i], path)) loaded++;
  }
  SDL_AudioSpec want;
  SDL_zero(want);
  want.freq = AUDIO_RATE;
  want.format = AUDIO_S16SYS;
  want.channels = 1;
  want.samples = 512;
  want.callback = callback;
  want.userdata = a;
  SDL_AudioSpec have;
  a->dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (!a->dev || loaded == 0) {
    fprintf(stderr, "audio unavailable (%s); continuing silently\n", a->dev ? "no wav files, run `make audio`" : SDL_GetError());
    audio_destroy(a);
    return NULL;
  }
  a->music_gain = 0.5f;
  a->sfx_gain = 0.8f;
  SDL_PauseAudioDevice(a->dev, 0);
  return a;
}

void audio_destroy(Audio* a) {
  if (!a) return;
  if (a->dev) SDL_CloseAudioDevice(a->dev);
  for (int i = 0; i < SFX_COUNT; i++) free(a->sfx[i].data);
  for (int i = 0; i < MUS_COUNT; i++) free(a->mus[i].data);
  free(a);
}

void audio_play(Audio* a, Sfx s) {
  if (!a || s <= SFX_NONE || s >= SFX_COUNT || !a->sfx[s].data) return;
  SDL_LockAudioDevice(a->dev);
  int slot = -1, oldest = 0;
  uint32_t best = 0;
  int same = 0;
  for (int v = 0; v < VOICES; v++) {
    if (!a->voices[v].clip) { if (slot < 0) slot = v; continue; }
    if (a->voices[v].clip == &a->sfx[s]) same++;
    if (a->voices[v].pos > best) { best = a->voices[v].pos; oldest = v; }
  }
  if (same < 3) {
    if (slot < 0) slot = oldest;
    a->voices[slot] = (Voice){&a->sfx[s], 0, (s == SFX_SHOT || s == SFX_ENEMY_SHOT || s == SFX_HIT) ? 0.55f : 1.0f};
  }
  SDL_UnlockAudioDevice(a->dev);
}

void audio_music(Audio* a, Music m) {
  if (!a || m == a->music_id) return;
  SDL_LockAudioDevice(a->dev);
  a->music_id = m;
  a->music = (m > MUS_NONE && m < MUS_COUNT && a->mus[m].data) ? &a->mus[m] : NULL;
  a->music_pos = 0;
  SDL_UnlockAudioDevice(a->dev);
}

void audio_set_volume(Audio* a, int music_vol, int sfx_vol, bool muted) {
  if (!a) return;
  SDL_LockAudioDevice(a->dev);
  a->music_gain = muted ? 0 : (float)music_vol / 10.0f * 0.6f;
  a->sfx_gain = muted ? 0 : (float)sfx_vol / 10.0f * 0.9f;
  SDL_UnlockAudioDevice(a->dev);
}
