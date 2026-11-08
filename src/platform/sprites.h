#ifndef PLATFORM_SPRITES_H_
#define PLATFORM_SPRITES_H_
#include "platform/sprite_table.h"

typedef enum {
#define X(name, w, h, frames) SPR_##name,
  SPRITE_LIST(X)
#undef X
  SPR_COUNT
} SpriteId;

#define TILES_PER_THEME_SPR 33
/** Sprite id for a terrain tile, from theme (0..2) and local index (0..32). */
#define SPR_TILE(theme, local) ((SpriteId)(SPR_tile_t0_base0 + (theme) * TILES_PER_THEME_SPR + (local)))
#endif
