#include "platform/renderer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arcade_font.h"
#include "pixel_font.h"
#include "texture.h"

const SpriteStyle SPRITE_DEFAULT = {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, false, false, false};

#define MAX_FRAMES 95
#define RAD_TO_DEG 57.29577951308232f

typedef struct {
  int frames;
  rect_t rect[MAX_FRAMES];
} SpriteInfo;

struct Renderer {
  graphics_context_ptr gfx;
  texture_t atlas;      /* alpha-blended copy of the atlas */
  texture_t atlas_add;  /* the same pixels, blended by adding */
  SpriteInfo sprites[SPR_COUNT];
  rect_t white;
  arcade_font_t font;  /* the engine's arcade font: all game text */
  pixel_font_t small;  /* the engine's 5x7 pixel font: debug overlay only */
  float offset_x;       /* where the playfield starts inside the engine's view */
  float shake_x, shake_y;
};

static const char* const kNames[SPR_COUNT] = {
#define X(name, w, h, frames) #name,
    SPRITE_LIST(X)
#undef X
};
static const int kFrames[SPR_COUNT] = {
#define X(name, w, h, frames) frames,
    SPRITE_LIST(X)
#undef X
};

static bool load_atlas_metadata(Renderer* r, const char* path) {
  FILE* f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "cannot open %s (run `make assets`)\n", path);
    return false;
  }
  typedef struct { char name[64]; rect_t rc; } Entry;
  Entry* entries = (Entry*)malloc(sizeof(Entry) * 1024);
  int n = 0;
  char line[256];
  while (fgets(line, sizeof line, f) && n < 1024) {
    if (line[0] == '#') continue;
    Entry* e = &entries[n];
    if (sscanf(line, "%63s %d %d %d %d", e->name, &e->rc.x, &e->rc.y, &e->rc.w, &e->rc.h) == 5) n++;
  }
  fclose(f);
  bool ok = true;
  for (int i = 0; i < SPR_COUNT; i++) {
    r->sprites[i].frames = kFrames[i];
    for (int fr = 0; fr < kFrames[i]; fr++) {
      char want[80];
      if (kFrames[i] == 1) snprintf(want, sizeof want, "%s", kNames[i]);
      else snprintf(want, sizeof want, "%s.%d", kNames[i], fr);
      bool found = false;
      for (int k = 0; k < n; k++) {
        if (strcmp(entries[k].name, want) == 0) {
          r->sprites[i].rect[fr] = entries[k].rc;
          found = true;
          break;
        }
      }
      if (!found) {
        fprintf(stderr, "atlas is missing sprite %s\n", want);
        ok = false;
      }
    }
  }
  for (int k = 0; k < n; k++) {
    if (strcmp(entries[k].name, "_white") == 0) r->white = make_rect(entries[k].rc.x + 1, entries[k].rc.y + 1, 2, 2);
  }
  free(entries);
  return ok;
}

/* Reads the cooked atlas: u32 width, u32 height (little endian), then width*height RGBA pixels. */
static uint32_t* read_pixels(const char* path, int* w, int* h) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    fprintf(stderr, "cannot open %s (run `make`)\n", path);
    return NULL;
  }
  uint8_t head[8];
  if (fread(head, 1, 8, f) != 8) { fclose(f); return NULL; }
  *w = (int)(head[0] | head[1] << 8 | head[2] << 16 | (uint32_t)head[3] << 24);
  *h = (int)(head[4] | head[5] << 8 | head[6] << 16 | (uint32_t)head[7] << 24);
  size_t count = (size_t)*w * (size_t)*h;
  uint32_t* px = (uint32_t*)malloc(count * sizeof(uint32_t));
  if (px && fread(px, sizeof(uint32_t), count, f) != count) { free(px); px = NULL; }
  fclose(f);
  return px;
}

/* The engine's arcade font sheet has an opaque black background and load_arcade_font keeps it. So the sheet is loaded
   with the engine's colour-key loader (black becomes transparent) into the same bitmap-font structure, with the
   parameters the engine uses for this sheet (8x7 glyphs, 8 px rows, 32 px between colours). */
static arcade_font_t load_transparent_arcade_font(graphics_context_ptr gfx, const char* sheet_path) {
  arcade_font_t font;
  memset(&font, 0, sizeof font);
  font.bitmap_font.texture = load_texture_with_colorkey(gfx, sheet_path, 0, 0, 0);
  font.bitmap_font.char_width = 8;
  font.bitmap_font.char_height = 7;
  font.bitmap_font.row_spacing = 8;
  font.bitmap_font.color_offset = 32;
  return font;
}

Renderer* renderer_create(graphics_context_ptr gfx, const char* assets_dir, const char* cooked_dir, const char* font_sheet) {
  Renderer* r = (Renderer*)calloc(1, sizeof *r);
  if (!r) return NULL;
  r->gfx = gfx;
  char path[1100];
  snprintf(path, sizeof path, "%s/atlas.txt", assets_dir);
  bool ok = load_atlas_metadata(r, path);
  snprintf(path, sizeof path, "%s/atlas.rgba", cooked_dir);
  int w = 0, h = 0;
  uint32_t* px = ok ? read_pixels(path, &w, &h) : NULL;
  if (px) {
    r->atlas = texture_from_pixels(gfx, px, w, h, false, false);
    r->atlas_add = texture_from_pixels(gfx, px, w, h, false, true);
  }
  free(px);
  r->font = load_transparent_arcade_font(gfx, font_sheet);
  r->small = build_pixel_font(gfx);
  if (!texture_valid(&r->atlas) || !texture_valid(&r->atlas_add) || !texture_valid(&r->font.bitmap_font.texture)) {
    renderer_destroy(r);
    return NULL;
  }
  return r;
}

void renderer_destroy(Renderer* r) {
  if (!r) return;
  if (texture_valid(&r->atlas)) free_texture(&r->atlas);
  if (texture_valid(&r->atlas_add)) free_texture(&r->atlas_add);
  free_arcade_font(&r->font);
  free_pixel_font(&r->small);
  free(r);
}

void renderer_begin(Renderer* r) {
  begin_frame(r->gfx, 6, 6, 12);
  r->offset_x = floorf((graphics_view_width(r->gfx) - SCREEN_W) * 0.5f * ART_SCALE) / ART_SCALE;
  r->shake_x = r->shake_y = 0;
}

void renderer_end(Renderer* r) {
  float vw = graphics_view_width(r->gfx);
  float right = r->offset_x + SCREEN_W;
  if (r->offset_x > 0.0f) draw_filled_rect(r->gfx, 0, 0, r->offset_x, SCREEN_H, COLOR(6, 6, 12));
  if (vw > right) draw_filled_rect(r->gfx, right, 0, vw - right, SCREEN_H, COLOR(6, 6, 12));
}

void renderer_translate(Renderer* r, float dx, float dy) {
  r->shake_x = dx;
  r->shake_y = dy;
}

static uint8_t to_byte(float v) { return (uint8_t)(fminf(fmaxf(v, 0.0f), 1.0f) * 255.0f + 0.5f); }

void renderer_sprite(Renderer* r, SpriteId id, int frame, float x, float y, const SpriteStyle* st) {
  const SpriteInfo* s = &r->sprites[id];
  if (frame < 0) frame = 0;
  if (frame >= s->frames) frame = s->frames - 1;
  const rect_t* rc = &s->rect[frame];
  float w = (float)rc->w * st->scale / ART_SCALE, h = (float)rc->h * st->scale / ART_SCALE;
  /* snap the centre to the art's texel grid so hi-res sprites never shimmer */
  float cx = roundf((x + r->shake_x + r->offset_x) * ART_SCALE) / ART_SCALE;
  float cy = roundf((y + r->shake_y) * ART_SCALE) / ART_SCALE;
  draw_options_t opt;
  memset(&opt, 0, sizeof opt);
  opt.flip = st->flip_x;
  opt.angle = st->rot * RAD_TO_DEG;
  if (st->flip_y) { /* the engine flips left to right only: flipping up-down is that plus a half turn */
    opt.flip = !opt.flip;
    opt.angle += 180.0f;
  }
  opt.alpha = st->alpha > 0.0f ? st->alpha : 0.001f;
  if (st->r != 1.0f || st->g != 1.0f || st->b != 1.0f) {
    opt.tinted = true;
    opt.r = to_byte(st->r);
    opt.g = to_byte(st->g);
    opt.b = to_byte(st->b);
  }
  render_sprite(r->gfx, st->additive ? &r->atlas_add : &r->atlas, rc, make_frect(cx - w * 0.5f, cy - h * 0.5f, w, h), &opt);
}

void renderer_rect(Renderer* r, float x, float y, float w, float h, float cr, float cg, float cb, float ca) {
  draw_filled_rect_alpha(r->gfx, x + r->shake_x + r->offset_x, y + r->shake_y, w, h,
                         COLOR(to_byte(cr), to_byte(cg), to_byte(cb)), to_byte(ca));
}

/* The arcade font has A-Z, 0-9, ! / - and space, in eight fixed colours. Anything else is mapped to the nearest. */
static char arcade_char(char c) {
  if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
  if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '!' || c == '/' || c == '-' || c == ' ') return c;
  if (c == '<' || c == '>' || c == '+') return '-';
  return ' ';
}

static font_color_t arcade_colour(float cr, float cg, float cb) {
  static const struct { font_color_t id; float r, g, b; } kColours[] = {
      {FONT_COLOR_WHITE, 1.0f, 1.0f, 1.0f}, {FONT_COLOR_RED, 1.0f, 0.25f, 0.25f}, {FONT_COLOR_PINK, 1.0f, 0.55f, 0.8f},
      {FONT_COLOR_CYAN, 0.35f, 0.9f, 1.0f}, {FONT_COLOR_GOLD, 1.0f, 0.8f, 0.25f}, {FONT_COLOR_PEACH, 1.0f, 0.7f, 0.5f},
      {FONT_COLOR_YELLOW, 1.0f, 1.0f, 0.3f}, {FONT_COLOR_GREEN, 0.35f, 1.0f, 0.45f}};
  font_color_t best = FONT_COLOR_WHITE;
  float best_d = 1e9f;
  for (size_t i = 0; i < sizeof kColours / sizeof kColours[0]; i++) {
    float dr = cr - kColours[i].r, dg = cg - kColours[i].g, db = cb - kColours[i].b;
    float d = dr * dr + dg * dg + db * db;
    if (d < best_d) { best_d = d; best = kColours[i].id; }
  }
  return best;
}

int renderer_text_width(const char* s) { return (int)strlen(s) * 8; }

void renderer_text(Renderer* r, const char* s, float x, float y, float cr, float cg, float cb, float alpha) {
  char buf[128];
  size_t n = 0;
  for (; s[n] && n < sizeof buf - 1; n++) buf[n] = arcade_char(s[n]);
  buf[n] = 0;
  int a = (int)(fminf(fmaxf(alpha, 0.0f), 1.0f) * 255.0f + 0.5f);
  render_arcade_text_scaled_alpha(&r->font, r->gfx, buf, (int)floorf(x + r->shake_x + r->offset_x), (int)floorf(y + r->shake_y),
                                  arcade_colour(cr, cg, cb), 1, a);
}

void renderer_text_small(Renderer* r, const char* s, float x, float y, float cr, float cg, float cb) {
  render_pixel_text(&r->small, r->gfx, x + r->offset_x, y, s, 1.0f, to_byte(cr), to_byte(cg), to_byte(cb));
}

int renderer_frame_count(const Renderer* r, SpriteId id) { return r->sprites[id].frames; }

bool renderer_save_bmp(Renderer* r, const char* path) { return save_frame_bmp(r->gfx, path); }

void renderer_present(Renderer* r) { present_frame(r->gfx); }
