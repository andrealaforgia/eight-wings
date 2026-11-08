/**
 * @file audio.h
 * @brief Game audio on the sdl-2d-game-engine's mixer: sound effects in engine sound slots, music through
 *        the engine's music-source callback. All sounds are the generated WAV files in assets/audio.
 */
#ifndef PLATFORM_GAME_AUDIO_H_
#define PLATFORM_GAME_AUDIO_H_
#include "game/types.h"

typedef struct Audio Audio;
/** `volume` is the engine master volume (0..128). Returns NULL when audio is unavailable; callers tolerate that. */
Audio* audio_create(const char* asset_dir, int volume);
void audio_destroy(Audio* a);
void audio_play(Audio* a, Sfx s);
void audio_music(Audio* a, Music m);
void audio_set_volume(Audio* a, int music_vol, int sfx_vol, bool muted);
#endif
