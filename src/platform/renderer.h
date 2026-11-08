/**
 * @file renderer.h
 * @brief Sprite drawing for the 256x224 playfield, on the sdl-2d-game-engine.
 *
 * Everything is drawn through the engine's graphics context and texture API (render_sprite,
 * draw_filled_rect_alpha, begin_frame). The engine fits a view of fixed height (224 units) to the window;
 * the playfield is centred in it and any extra width becomes dark side bars.
 */
#ifndef PLATFORM_RENDERER_H_
#define PLATFORM_RENDERER_H_

#include <stdbool.h>

#include "game/types.h"
#include "graphics.h"
#include "platform/sprites.h"

typedef struct {
  float alpha;       /* 0..1 */
  float r, g, b;     /* colour multiplier 0..1, 1 = unchanged */
  float rot;         /* radians, clockwise */
  float scale;       /* 1 = native */
  bool flip_x, flip_y;
  bool additive;     /* draws from the additive copy of the atlas (glows, hit flashes) */
} SpriteStyle;

/* Art is authored at ART_SCALE texels per logical pixel (hi-res sprites on the 256x224 playfield). */
#define ART_SCALE 4
extern const SpriteStyle SPRITE_DEFAULT;

typedef struct Renderer Renderer;

/** Loads assets_dir/atlas.txt and the cooked atlas pixels (cooked_dir/atlas.rgba) into engine textures. */
Renderer* renderer_create(graphics_context_ptr gfx, const char* assets_dir, const char* cooked_dir);
void renderer_destroy(Renderer* r);
/** Starts a frame: the engine fits the view to the window and clears it. */
void renderer_begin(Renderer* r);
/** Ends a frame: covers whatever spilled outside the playfield with the side bars. */
void renderer_end(Renderer* r);
void renderer_translate(Renderer* r, float dx, float dy); /* screen shake */
/** Draw frame `frame` of a sprite with its centre at (x, y), in playfield units. */
void renderer_sprite(Renderer* r, SpriteId id, int frame, float x, float y, const SpriteStyle* style);
void renderer_rect(Renderer* r, float x, float y, float w, float h, float cr, float cg, float cb, float ca);
/** Draw text with the 8x8 bitmap font; (x,y) is the top-left of the first glyph. */
void renderer_text(Renderer* r, const char* s, float x, float y, float cr, float cg, float cb, float alpha);
int renderer_text_width(const char* s);
/** Shows the finished frame. */
void renderer_present(Renderer* r);
/** Saves the frame drawn so far as a BMP. */
bool renderer_save_bmp(Renderer* r, const char* path);
int renderer_frame_count(const Renderer* r, SpriteId id);

#endif
