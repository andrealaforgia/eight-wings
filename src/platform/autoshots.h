/** Automated, scripted play session that saves real screenshots of the running game (not mock-ups). */
#ifndef PLATFORM_AUTOSHOTS_H_
#define PLATFORM_AUTOSHOTS_H_
#include <SDL.h>

#include "game/game.h"
#include "platform/renderer.h"
/** Returns 0 when every expected screenshot was captured. */
int autoshots_run(Renderer* r, SDL_Window* win, Game* g, const char* out_dir, uint32_t seed);
#endif
