/** Small SDL2 software mixer: 16 SFX voices + one looping music stream, all loaded from generated WAVs. */
#ifndef PLATFORM_AUDIO_H_
#define PLATFORM_AUDIO_H_
#include "game/types.h"

typedef struct Audio Audio;
/** Returns NULL when audio is unavailable; callers must tolerate that (the game runs silently). */
Audio* audio_create(const char* asset_dir);
void audio_destroy(Audio* a);
void audio_play(Audio* a, Sfx s);
void audio_music(Audio* a, Music m);
void audio_set_volume(Audio* a, int music_vol, int sfx_vol, bool muted);
#endif
