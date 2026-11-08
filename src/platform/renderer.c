#include "platform/renderer.h"

#include <SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/gl.h"

const SpriteStyle SPRITE_DEFAULT = {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, false, false, false};

#define MAX_QUADS 6000
#define MAX_FRAMES 95

typedef struct {
  float x, y, u, v;
  uint8_t c[4];
} Vertex;

typedef struct {
  int frames;
  int rect[MAX_FRAMES][4];
} SpriteInfo;

struct Renderer {
  SDL_Window* window;
  SDL_GLContext gl;
  GLuint tex;
  int tex_w, tex_h;
  SpriteInfo sprites[SPR_COUNT];
  int white[4];
  Vertex* verts;
  int nverts;
  bool additive;
  int vp_x, vp_y, vp_w, vp_h, scale;
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
  typedef struct { char name[64]; int x, y, w, h; } Entry;
  Entry* entries = (Entry*)malloc(sizeof(Entry) * 1024);
  int n = 0;
  char line[256];
  while (fgets(line, sizeof line, f) && n < 1024) {
    if (line[0] == '#') continue;
    if (sscanf(line, "%63s %d %d %d %d", entries[n].name, &entries[n].x, &entries[n].y, &entries[n].w, &entries[n].h) == 5) n++;
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
          r->sprites[i].rect[fr][0] = entries[k].x;
          r->sprites[i].rect[fr][1] = entries[k].y;
          r->sprites[i].rect[fr][2] = entries[k].w;
          r->sprites[i].rect[fr][3] = entries[k].h;
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
    if (strcmp(entries[k].name, "_white") == 0) {
      r->white[0] = entries[k].x + 1; r->white[1] = entries[k].y + 1; r->white[2] = 2; r->white[3] = 2;
    }
  }
  free(entries);
  return ok;
}

Renderer* renderer_create(SDL_Window* window, const char* asset_dir) {
  Renderer* r = (Renderer*)calloc(1, sizeof *r);
  if (!r) return NULL;
  r->window = window;
  r->gl = SDL_GL_CreateContext(window);
  if (!r->gl) {
    fprintf(stderr, "GL context: %s\n", SDL_GetError());
    free(r);
    return NULL;
  }
  SDL_GL_SetSwapInterval(1);
  char path[1024];
  snprintf(path, sizeof path, "%s/atlas.png", asset_dir);
  SDL_Surface* raw = IMG_Load(path);
  if (!raw) {
    fprintf(stderr, "cannot load %s: %s (run `make assets`)\n", path, IMG_GetError());
    SDL_GL_DeleteContext(r->gl);
    free(r);
    return NULL;
  }
  SDL_Surface* rgba = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_ABGR8888, 0); /* bytes R,G,B,A in memory */
  SDL_FreeSurface(raw);
  r->tex_w = rgba->w;
  r->tex_h = rgba->h;
  glGenTextures(1, &r->tex);
  glBindTexture(GL_TEXTURE_2D, r->tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, rgba->pitch / 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  SDL_FreeSurface(rgba);
  snprintf(path, sizeof path, "%s/atlas.txt", asset_dir);
  if (!load_atlas_metadata(r, path)) {
    renderer_destroy(r);
    return NULL;
  }
  r->verts = (Vertex*)malloc(sizeof(Vertex) * MAX_QUADS * 4);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  return r;
}

void renderer_destroy(Renderer* r) {
  if (!r) return;
  if (r->tex) glDeleteTextures(1, &r->tex);
  if (r->gl) SDL_GL_DeleteContext(r->gl);
  free(r->verts);
  free(r);
}

static void flush(Renderer* r) {
  if (r->nverts == 0) return;
  if (r->additive) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &r->verts[0].x);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &r->verts[0].u);
  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), r->verts[0].c);
  glDrawArrays(GL_QUADS, 0, r->nverts);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  r->nverts = 0;
}

void renderer_begin(Renderer* r, int win_w, int win_h) {
  int dw, dh;
  SDL_GL_GetDrawableSize(r->window, &dw, &dh);
  (void)win_w; (void)win_h;
  /* Largest scale that fits. Integer scales keep pixel art perfectly even; if rounding down would waste more
     than 10% of the window (typical on HiDPI screens), use the exact fractional fit instead. */
  float fit = fminf((float)dw / SCREEN_W, (float)dh / SCREEN_H);
  float whole = floorf(fit);
  float use = (whole >= 1.0f && (fit - whole) / fit <= 0.10f) ? whole : fit;
  r->scale = (int)(use + 0.5f);
  r->vp_w = (int)(SCREEN_W * use);
  r->vp_h = (int)(SCREEN_H * use);
  r->vp_x = (dw - r->vp_w) / 2;
  r->vp_y = (dh - r->vp_h) / 2;
  glViewport(0, 0, dw, dh);
  glDisable(GL_SCISSOR_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(r->vp_x, r->vp_y, r->vp_w, r->vp_h);
  glEnable(GL_SCISSOR_TEST);
  glScissor(r->vp_x, r->vp_y, r->vp_w, r->vp_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, SCREEN_W, SCREEN_H, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glBindTexture(GL_TEXTURE_2D, r->tex);
  r->nverts = 0;
  r->additive = false;
  r->shake_x = r->shake_y = 0;
}

void renderer_end(Renderer* r) {
  flush(r);
  glDisable(GL_SCISSOR_TEST);
}

void renderer_translate(Renderer* r, float dx, float dy) {
  r->shake_x = dx;
  r->shake_y = dy;
}

static void push_quad(Renderer* r, bool additive, const float px[8], const float uv[4], float cr, float cg, float cb, float ca) {
  if (additive != r->additive) { flush(r); r->additive = additive; }
  if (r->nverts + 4 > MAX_QUADS * 4) flush(r);
  uint8_t c[4] = {(uint8_t)(fminf(cr, 1) * 255), (uint8_t)(fminf(cg, 1) * 255), (uint8_t)(fminf(cb, 1) * 255), (uint8_t)(fminf(ca, 1) * 255)};
  float u0 = uv[0], v0 = uv[1], u1 = uv[2], v1 = uv[3];
  float us[4] = {u0, u1, u1, u0}, vs[4] = {v0, v0, v1, v1};
  for (int i = 0; i < 4; i++) {
    Vertex* v = &r->verts[r->nverts++];
    v->x = px[i * 2] + r->shake_x;
    v->y = px[i * 2 + 1] + r->shake_y;
    v->u = us[i];
    v->v = vs[i];
    memcpy(v->c, c, 4);
  }
}

void renderer_sprite(Renderer* r, SpriteId id, int frame, float x, float y, const SpriteStyle* st) {
  const SpriteInfo* s = &r->sprites[id];
  if (frame < 0) frame = 0;
  if (frame >= s->frames) frame = s->frames - 1;
  const int* rc = s->rect[frame];
  float w = (float)rc[2] * st->scale / ART_SCALE, h = (float)rc[3] * st->scale / ART_SCALE;
  float cx = roundf(x * ART_SCALE) / ART_SCALE, cy = roundf(y * ART_SCALE) / ART_SCALE;
  float u0 = (float)rc[0] / (float)r->tex_w, v0 = (float)rc[1] / (float)r->tex_h;
  float u1 = (float)(rc[0] + rc[2]) / (float)r->tex_w, v1 = (float)(rc[1] + rc[3]) / (float)r->tex_h;
  if (st->flip_x) { float t = u0; u0 = u1; u1 = t; }
  if (st->flip_y) { float t = v0; v0 = v1; v1 = t; }
  float uv[4] = {u0, v0, u1, v1};
  float hw = w * 0.5f, hh = h * 0.5f;
  float px[8];
  if (st->rot == 0.0f) {
    float l = roundf((cx - hw) * ART_SCALE) / ART_SCALE, t = roundf((cy - hh) * ART_SCALE) / ART_SCALE;
    px[0] = l; px[1] = t; px[2] = l + w; px[3] = t; px[4] = l + w; px[5] = t + h; px[6] = l; px[7] = t + h;
  } else {
    float cs = cosf(st->rot), sn = sinf(st->rot);
    float lx[4] = {-hw, hw, hw, -hw}, ly[4] = {-hh, -hh, hh, hh};
    for (int i = 0; i < 4; i++) {
      px[i * 2] = cx + lx[i] * cs - ly[i] * sn;
      px[i * 2 + 1] = cy + lx[i] * sn + ly[i] * cs;
    }
  }
  push_quad(r, st->additive, px, uv, st->r, st->g, st->b, st->alpha);
}

void renderer_rect(Renderer* r, float x, float y, float w, float h, float cr, float cg, float cb, float ca) {
  float u = ((float)r->white[0] + 1.0f) / (float)r->tex_w, v = ((float)r->white[1] + 1.0f) / (float)r->tex_h;
  float uv[4] = {u, v, u, v};
  float px[8] = {x, y, x + w, y, x + w, y + h, x, y + h};
  push_quad(r, false, px, uv, cr, cg, cb, ca);
}

int renderer_text_width(const char* s) { return (int)strlen(s) * 8; }

void renderer_text(Renderer* r, const char* s, float x, float y, float cr, float cg, float cb, float alpha) {
  SpriteStyle st = SPRITE_DEFAULT;
  st.r = cr; st.g = cg; st.b = cb; st.alpha = alpha;
  for (; *s; s++, x += 8) {
    int c = (unsigned char)*s;
    if (c < 32 || c > 126) c = '?';
    if (c == ' ') continue;
    renderer_sprite(r, SPR_font, c - 32, x + 4, y + 4, &st);
  }
}

int renderer_frame_count(const Renderer* r, SpriteId id) { return r->sprites[id].frames; }

void renderer_viewport_info(const Renderer* r, int* x, int* y, int* w, int* h, int* scale) {
  *x = r->vp_x; *y = r->vp_y; *w = r->vp_w; *h = r->vp_h; *scale = r->scale;
}

bool renderer_read_pixels(Renderer* r, const char* png_path) {
  flush(r);
  int w = r->vp_w, h = r->vp_h;
  uint8_t* buf = (uint8_t*)malloc((size_t)w * (size_t)h * 4);
  if (!buf) return false;
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(r->vp_x, r->vp_y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);
  /* flip rows */
  uint8_t* row = (uint8_t*)malloc((size_t)w * 4);
  for (int y = 0; y < h / 2; y++) {
    memcpy(row, buf + (size_t)y * w * 4, (size_t)w * 4);
    memcpy(buf + (size_t)y * w * 4, buf + (size_t)(h - 1 - y) * w * 4, (size_t)w * 4);
    memcpy(buf + (size_t)(h - 1 - y) * w * 4, row, (size_t)w * 4);
  }
  free(row);
  SDL_Surface* s = SDL_CreateRGBSurfaceWithFormatFrom(buf, w, h, 32, w * 4, SDL_PIXELFORMAT_ABGR8888);
  bool ok = s && IMG_SavePNG(s, png_path) == 0;
  if (s) SDL_FreeSurface(s);
  free(buf);
  return ok;
}
