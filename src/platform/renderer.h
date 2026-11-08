/** Low-level OpenGL sprite batcher on a 256x224 logical canvas (nearest-neighbour, integer scaling). */
#ifndef PLATFORM_RENDERER_H_
#define PLATFORM_RENDERER_H_

#include <SDL.h>

#include "game/types.h"
#include "platform/sprites.h"

typedef struct {
  float alpha;       /* 0..1 (0 treated as 1 when using SPRITE_DEFAULT) */
  float r, g, b;     /* colour multiplier, 1 = unchanged */
  float rot;         /* radians, clockwise */
  float scale;       /* 1 = native */
  bool flip_x, flip_y;
  bool additive;
} SpriteStyle;

/* Art is authored at ART_SCALE texels per logical pixel (hi-res sprites on the 256x224 playfield). */
#define ART_SCALE 4
extern const SpriteStyle SPRITE_DEFAULT;

typedef struct Renderer Renderer;

Renderer* renderer_create(SDL_Window* window, const char* asset_dir);
void renderer_destroy(Renderer* r);
/** Starts a frame: clears the window, sets the letterboxed integer-scaled viewport. */
void renderer_begin(Renderer* r, int win_w, int win_h);
void renderer_end(Renderer* r);  /* flush */
void renderer_translate(Renderer* r, float dx, float dy); /* screen shake */
/** Draw frame `frame` of a sprite with its centre at (x, y). Coordinates snap to the pixel grid. */
void renderer_sprite(Renderer* r, SpriteId id, int frame, float x, float y, const SpriteStyle* style);
void renderer_rect(Renderer* r, float x, float y, float w, float h, float cr, float cg, float cb, float ca);
/** Draw text with the 8x8 bitmap font; (x,y) is the top-left of the first glyph. */
void renderer_text(Renderer* r, const char* s, float x, float y, float cr, float cg, float cb, float alpha);
int renderer_text_width(const char* s);
bool renderer_read_pixels(Renderer* r, const char* png_path);  /* saves the 256x224-scaled framebuffer region as PNG */
int renderer_frame_count(const Renderer* r, SpriteId id);
void renderer_viewport_info(const Renderer* r, int* x, int* y, int* w, int* h, int* scale);

#endif
