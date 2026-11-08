/** Draws a Game snapshot with the renderer. Pure presentation: never mutates the simulation. */
#ifndef PLATFORM_SCENE_H_
#define PLATFORM_SCENE_H_
#include "game/game.h"
#include "platform/renderer.h"

void scene_draw(Renderer* r, const Game* g, long frame);
#endif
