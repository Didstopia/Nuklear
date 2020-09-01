/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2017 by Micha Mettke
 *
 * Actualizacion por Jhoson Ozuna(slam) - hbiblia@g
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_SDL_H_
#define NK_SDL_H_

#ifdef SDL_INCLUDE_AT_ROOT
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#endif

typedef struct NKSdlFont NKSdlFont;
NK_API struct nk_context *nk_sdl_init(SDL_Window *win, SDL_Renderer *renderer);
NK_API int nk_sdl_handle_event(SDL_Event *evt);
// NK_API void nk_sdl_render(void);
NK_API void nk_sdl_render_begin(void);
NK_API SDL_Texture *nk_sdl_render_end(void);
NK_API void nk_sdl_shutdown(void);

NK_API void nk_sdl_font_create_from_file(const char *file_name, int font_size, int flags);
NK_API void nk_sdl_font_del(void);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_SDL2_IMPLEMENTATION

#ifndef NK_SDL_TEXT_MAX
#define NK_SDL_TEXT_MAX 256
#endif

struct nk_scale {
  float x;
  float y;
};

static struct nk_sdl {
  SDL_Window *win;
  SDL_Renderer *renderer;
  SDL_Texture *target;
  struct nk_scale scale;
  struct nk_context ctx;
  struct nk_buffer cmds;
  // font data
  TTF_Font *ttf_font;
  struct nk_user_font *user_font;
  int font_height;
} sdl;

NK_API void nk_sdl_font_create_from_file(const char *file_name, int font_size, int flags) {
  // SDL_TTF should already be initialized outside of this library
  // if (TTF_Init() != 0) {
  //   fprintf(stdout, "Unable to initialize SDL_TTF\n");
  // }

  sdl.ttf_font = TTF_OpenFont(file_name, font_size);
  if (sdl.ttf_font == NULL) {
    fprintf(stdout, "Unable to load font file: %s\n", file_name);
  }
}

NK_API void nk_sdl_font_del(void) {
  if (!sdl.ttf_font) return;
  TTF_CloseFont(sdl.ttf_font);
}

void sdl_draw_text(TTF_Font *font, const char *str, int x, int y, struct nk_color c) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, str, (SDL_Color){c.r, c.g, c.b});
  SDL_Texture *texture = SDL_CreateTextureFromSurface(sdl.renderer, surface);
  int texW = 0, texH = 0;
  SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
  SDL_Rect dstrect = {x, y, texW, texH};
  SDL_RenderCopy(sdl.renderer, texture, NULL, &dstrect);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

void sdl_draw_rect(int x, int y, int w, int h, int rounding, struct nk_color color) {
  roundedRectangleRGBA(sdl.renderer, x, y, (x + w), (y + h), rounding, color.r, color.g, color.b,
                       color.a);
}

void sdl_draw_rect_fill(int x, int y, int w, int h, int rounding, struct nk_color color) {
  roundedBoxRGBA(sdl.renderer, x, y, (x + w), (y + h), NK_MAX(rounding, 2), color.r, color.g,
                 color.b, color.a);
}

void interpolate_color(struct nk_color c1, struct nk_color c2, struct nk_color *result,
                       float fraction) {
  float r = c1.r + (c2.r - c1.r) * fraction;
  float g = c1.g + (c2.g - c1.g) * fraction;
  float b = c1.b + (c2.b - c1.b) * fraction;
  float a = c1.a + (c2.a - c1.a) * fraction;

  result->r = (nk_byte)NK_CLAMP(0, r, 255);
  result->g = (nk_byte)NK_CLAMP(0, g, 255);
  result->b = (nk_byte)NK_CLAMP(0, b, 255);
  result->a = (nk_byte)NK_CLAMP(0, a, 255);
}

void sdl_draw_rect_fill_multicolor(int x, int y, int w, int h, struct nk_color left,
                                   struct nk_color top, struct nk_color right,
                                   struct nk_color bottom) {
  struct nk_color X1, X2, Y;
  float fraction_x, fraction_y;
  int i, j;
  for (j = 0; j < h; j++) {
    fraction_y = ((float)j) / h;
    for (i = 0; i < w; i++) {
      fraction_x = ((float)i) / w;
      interpolate_color(left, top, &X1, fraction_x);
      interpolate_color(right, bottom, &X2, fraction_x);
      interpolate_color(X1, X2, &Y, fraction_y);
      pixelRGBA(sdl.renderer, x + i, y + j, Y.r, Y.g, Y.b, Y.a);
    }
  }
}

void sdl_draw_line(int x, int y, int x2, int y2, struct nk_color color) {
  lineRGBA(sdl.renderer, x, y, x2, y2, color.r, color.g, color.b, color.a);
}

void sdl_draw_ellipse(int x, int y, int rx, int ry, struct nk_color color) {
  ellipseRGBA(sdl.renderer, x, y, rx, ry, color.r, color.g, color.b, color.a);
}

void sdl_draw_ellipse_filled(int x, int y, int rx, int ry, struct nk_color color) {
  filledEllipseRGBA(sdl.renderer, x, y, rx, ry, color.r, color.g, color.b, color.a);
}

void sdl_draw_circle_filled(int x, int y, int rad, struct nk_color color) {
  filledCircleRGBA(sdl.renderer, x, y, rad, color.r, color.g, color.b, color.a);
}

void sdl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, struct nk_color color) {
  trigonRGBA(sdl.renderer, x1, y1, x2, y2, x3, y3, color.r, color.g, color.b, color.a);
}

void sdl_draw_filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3,
                              struct nk_color color) {
  filledTrigonRGBA(sdl.renderer, x1, y1, x2, y2, x3, y3, color.r, color.g, color.b, color.a);
}

void sdl_draw_polyline(const Sint16 *vx, const Sint16 *vy, int n, struct nk_color color) {
  polygonRGBA(sdl.renderer, vx, vy, n, color.r, color.g, color.b, color.a);
}

void sdl_draw_arc(int x, int y, int rad, int start, int end, struct nk_color color) {
  arcRGBA(sdl.renderer, x, y, rad, start, end, color.r, color.g, color.b, color.a);
}

void sdl_draw_filled_polygon(const Sint16 *vx, const Sint16 *vy, int n, struct nk_color color) {
  filledPolygonRGBA(sdl.renderer, vx, vy, n, color.r, color.g, color.b, color.a);
}

void sdl_draw_bezier(const Sint16 *vx, const Sint16 *vy, int n, int s, struct nk_color color) {
  bezierRGBA(sdl.renderer, vx, vy, n, s, color.r, color.g, color.b, color.a);
}

void sdl_draw_image(const struct nk_command_image *image, int x, int y, int w, int h) {
  SDL_Texture *t =
      SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h);
  SDL_SetTextureColorMod(t, image->col.r, image->col.g, image->col.b);

  auto imageHandlePointer = image->img.handle.ptr;
  auto imageHandlePointerCast = (int *)imageHandlePointer;
  // auto imageHandlePointerRef = &imageHandlePointerCast;

  const SDL_Rect destinationRect = {x, y, w, h};

  // SDL_QueryTexture(t, NULL, &image->img.handle.ptr, &w, &h);
  SDL_QueryTexture(t, NULL, imageHandlePointerCast, &w, &h);
  // SDL_UpdateTexture(t, &(SDL_Rect){x, y, w, h}, &image->img.handle.ptr,
  // sizeof(*image->img.handle.ptr));
  SDL_UpdateTexture(t, &destinationRect, imageHandlePointerCast, sizeof(&imageHandlePointerCast));
  // SDL_RenderCopy(sdl.renderer, t, NULL, &(SDL_Rect){x, y, w, h});
  SDL_RenderCopy(sdl.renderer, t, NULL, &destinationRect);
}

NK_API void nk_sdl_render_begin(void) { SDL_SetRenderTarget(sdl.renderer, sdl.target); }

// NK_API void nk_sdl_render(void) {
// NK_API void nk_sdl_render_end(void) {
NK_API SDL_Texture *nk_sdl_render_end(void) {
  SDL_RenderClear(sdl.renderer);
  const struct nk_command *cmd;
  nk_foreach(cmd, &sdl.ctx) {
    //    Uint32 color; temporalmente no tiene uso.
    switch (cmd->type) {
      case NK_COMMAND_NOP: {
        // Do nothing
      } break;
      case NK_COMMAND_SCISSOR: {
        // FIXME: This works perfectly in OpenGL, but it's a few pixels off with Metal
        // FIXME: (UPDATE) The clip rect size seems to be the same, so there's a
        //        chance that it's NOT the clipping that's the issue, but the rendering?
        //        Although disabling clipping will render perfectly fine, even on Metal..
        const struct nk_command_scissor *s = (const struct nk_command_scissor *)cmd;
        const SDL_Rect r = {(int)s->x, (int)s->y, (int)s->w, (int)s->h};
        // SDL_RenderSetClipRect(sdl.renderer,
        //                       &(SDL_Rect){(int)s->x, (int)s->y, (int)s->w, (int)s->h});
        SDL_RenderSetClipRect(sdl.renderer, &r);

      } break;
      case NK_COMMAND_LINE: {
        // TODO: Missing line thickness
        const struct nk_command_line *l = (const struct nk_command_line *)cmd;
        sdl_draw_line((float)l->begin.x, (float)l->begin.y, (float)l->end.x, (float)l->end.y,
                      l->color);
      } break;
      case NK_COMMAND_RECT: {
        // TODO: Missing line thickness
        const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
        sdl_draw_rect((float)r->x, (float)r->y, (float)r->w, (float)r->h, (float)r->rounding,
                      r->color);
      } break;
      case NK_COMMAND_RECT_FILLED: {
        // TODO: Missing line thickness
        const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
        sdl_draw_rect_fill((float)r->x, (float)r->y, (float)r->w, (float)r->h, (float)r->rounding,
                           r->color);
      } break;
      case NK_COMMAND_CIRCLE: {
        // TODO: Missing line thickness
        const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
        int xr, yr;
        xr = (float)c->w / 2;
        yr = (float)c->h / 2;
        sdl_draw_ellipse(((float)(c->x)) + xr, ((float)c->y) + yr, xr, yr, c->color);
      } break;
      case NK_COMMAND_CIRCLE_FILLED: {
        const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
        int xr, yr;
        xr = (float)c->w / 2;
        yr = (float)c->h / 2;
        sdl_draw_ellipse_filled(((float)(c->x)) + xr, ((float)c->y) + yr, xr, yr, c->color);
      } break;
      case NK_COMMAND_TRIANGLE: {
        // TODO: Missing line thickness
        const struct nk_command_triangle *t = (const struct nk_command_triangle *)cmd;
        sdl_draw_triangle((float)t->a.x, (float)t->a.y, (float)t->b.x, (float)t->b.y, (float)t->c.x,
                          (float)t->c.y, t->color);
      } break;
      case NK_COMMAND_TRIANGLE_FILLED: {
        const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
        sdl_draw_filled_triangle((float)t->a.x, (float)t->a.y, (float)t->b.x, (float)t->b.y,
                                 (float)t->c.x, (float)t->c.y, t->color);
      } break;
      case NK_COMMAND_POLYGON: {
        // TODO: Missing line thickness
        const struct nk_command_polygon *p = (const struct nk_command_polygon *)cmd;
        int i;
        float vertices[p->point_count * 2];
        for (i = 0; i < p->point_count; i++) {
          vertices[i * 2] = p->points[i].x;
          vertices[(i * 2) + 1] = p->points[i].y;
        }
        // sdl_draw_polyline((const float *)&vertices, (2 * sizeof(float)), (int)p->point_count,
        // p->color);
        sdl_draw_polyline((const Sint16 *)&vertices, (const Sint16 *)(2 * sizeof(Sint16)),
                          (int)p->point_count, p->color);
      } break;
      case NK_COMMAND_POLYGON_FILLED: {
        const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
        int i;
        float verticesx[p->point_count * 2];
        float verticesy[p->point_count * 2];
        for (i = 0; i < p->point_count; i++) {
          verticesx[i * 2] = p->points[i].x;
          verticesy[(i * 2) + 1] = p->points[i].y;
        }
        // sdl_draw_filled_polygon((const float *)&verticesx, (const float *)&verticesy,
        // (int)p->point_count, p->color);
        sdl_draw_filled_polygon((const Sint16 *)&verticesx, (const Sint16 *)&verticesy,
                                (int)p->point_count, p->color);
      } break;
      case NK_COMMAND_POLYLINE: {
        // TODO: Missing line thickness
        // FIXME: This is the same as NK_COMMAND_POLYGON, except this should have "line cap round",
        //        while NK_COMMAND_POLYGON should have "line cap closed"
        const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
        int i;
        float vertices[p->point_count * 2];
        for (i = 0; i < p->point_count; i++) {
          vertices[i * 2] = p->points[i].x;
          vertices[(i * 2) + 1] = p->points[i].y;
        }
        sdl_draw_polyline((const Sint16 *)&vertices, (const Sint16 *)(2 * sizeof(Sint16)),
                          (int)p->point_count, p->color);
      } break;
      case NK_COMMAND_TEXT: {
        const struct nk_command_text *t = (const struct nk_command_text *)cmd;
        TTF_Font *font = (TTF_Font *)t->font->userdata.ptr;
        sdl_draw_text(font, (const char *)t->string, (float)t->x, (float)t->y, t->foreground);
      } break;
      case NK_COMMAND_CURVE: {
        // TODO: Missing line thickness
        // FIXME: This is likely very much incorrect and broken (drawing a spline/curve)
        const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
        float points[8];
        points[0] = (float)q->begin.x;
        points[1] = (float)q->begin.y;
        points[2] = (float)q->ctrl[0].x;
        points[3] = (float)q->ctrl[0].y;
        points[4] = (float)q->ctrl[1].x;
        points[5] = (float)q->ctrl[1].y;
        points[6] = (float)q->end.x;
        points[7] = (float)q->end.y;
        const int point_count = 8;
        const int steps = 2;
        sdl_draw_bezier((const Sint16 *)&points, (const Sint16 *)(2 * sizeof(Sint16)), point_count,
                        steps, q->color);
      } break;
      case NK_COMMAND_ARC: {
        // TODO: Missing line thickness
        const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
        sdl_draw_arc((float)a->cx, (float)a->cy, (float)a->r, a->a[0], a->a[1], a->color);
      } break;
      case NK_COMMAND_IMAGE: {
        const struct nk_command_image *i = (const struct nk_command_image *)cmd;
        sdl_draw_image(i, (float)i->x, (float)i->y, (float)i->w, (float)i->h);
      } break;
      case NK_COMMAND_RECT_MULTI_COLOR: {
        // FIXME: This needs to draw 4 rects, instead of just the one we're drawing right now
        // FIXME: I bet we need this for the color picker..
        const struct nk_command_rect_multi_color *r =
            (const struct nk_command_rect_multi_color *)cmd;
        sdl_draw_rect_fill_multicolor((float)r->x, (float)r->y, (float)r->w, (float)r->h, r->left,
                                      r->top, r->right, r->bottom);
      } break;
      case NK_COMMAND_ARC_FILLED: {
        // FIXME: This is likely incorrect (we're not filling it!)
        const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
        sdl_draw_arc((float)a->cx, (float)a->cy, (float)a->r, a->a[0], a->a[1], a->color);
      } break;
      default:
        break;
    }
  }
  nk_clear(&sdl.ctx);
  SDL_SetRenderTarget(sdl.renderer, NULL);
  return sdl.target;
}

static void nk_sdl_clipboard_paste(nk_handle usr, struct nk_text_edit *edit) {
  const char *text = SDL_GetClipboardText();
  if (text) nk_textedit_paste(edit, text, nk_strlen(text));
  (void)usr;
}

static void nk_sdl_clipboard_copy(nk_handle usr, const char *text, int len) {
  char *str = 0;
  (void)usr;
  if (!len) return;
  str = (char *)malloc((size_t)len + 1);
  if (!str) return;
  memcpy(str, text, (size_t)len);
  str[len] = '\0';
  SDL_SetClipboardText(str);
  free(str);
}

static float nk_sdl_font_get_text_width(nk_handle handle, float height, const char *text, int len) {
  TTF_Font *font = (TTF_Font *)handle.ptr;
  if (!font || !text) {
    return 0;
  }
  /* We must copy into a new buffer with exact length null-terminated
     as nuklear uses variable size buffers and al_get_text_width doesn't
     accept a length, it infers length from null-termination
     (which is unsafe API design by allegro devs!) */
  char tmp_buffer[len + 1];
  strncpy((char *)&tmp_buffer, text, len);
  tmp_buffer[len] = '\0';

  int w, h;
  TTF_SizeText(font, tmp_buffer, &w, &h);
  return (float)w;
}

NK_API struct nk_context *nk_sdl_init(SDL_Window *win, SDL_Renderer *renderer) {
  // struct nk_user_font *font = &sdl.user_font;
  if (sdl.user_font == NULL) {
    sdl.user_font = (struct nk_user_font *)malloc(sizeof(struct nk_user_font));
  }
  struct nk_user_font *font = sdl.user_font;
  font->userdata = nk_handle_ptr(sdl.ttf_font);
  font->height = TTF_FontHeight(sdl.ttf_font);
  font->width = nk_sdl_font_get_text_width;

  // FIXME: This is not enough in case the window is resized or resolution changes!

  // Get the renderer logical size (used for the target texture)
  int w, h;
  // SDL_GetWindowSize(win, &w, &h);
  SDL_RenderGetLogicalSize(renderer, &w, &h);

  // FIXME: Nuklear, scaling and mouse positions are still somewhat flaky
  // (adjusting sliders is 50% off, due to 2x scaling)

  // Get the renderer scale
  SDL_RenderGetScale(renderer, &sdl.scale.x, &sdl.scale.y);

  sdl.win = win;
  sdl.renderer = renderer;
  sdl.target =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

  // Enable alpha blending (transparency)
  SDL_SetTextureBlendMode(sdl.target, SDL_BLENDMODE_BLEND);

  nk_init_default(&sdl.ctx, font);
  nk_buffer_init_default(&sdl.cmds);

  sdl.ctx.clip.copy = nk_sdl_clipboard_copy;
  sdl.ctx.clip.paste = nk_sdl_clipboard_paste;
  sdl.ctx.clip.userdata = nk_handle_ptr(0);

  return &sdl.ctx;
}

NK_API int nk_sdl_handle_event(SDL_Event *evt) {
  struct nk_context *ctx = &sdl.ctx;

  // FIXME: This doesn't work most of the time!
  //        Events swallowed by our console?

  /* optional grabbing behavior */
  // if (ctx->input.mouse.grab) {
  //   // FIXME: This is almost never called?
  //   SDL_SetRelativeMouseMode(SDL_TRUE);
  //   SDL_ShowCursor(SDL_ENABLE);
  //   ctx->input.mouse.grab = 0;
  // } else if (ctx->input.mouse.ungrab) {
  //   // FIXME: This is almost never called?
  //   int x = (int)ctx->input.mouse.prev.x;
  //   int y = (int)ctx->input.mouse.prev.y;
  //   SDL_SetRelativeMouseMode(SDL_FALSE);
  //   SDL_WarpMouseInWindow(sdl.win, x, y);
  //   SDL_ShowCursor(SDL_ENABLE);
  //   ctx->input.mouse.ungrab = 0;
  // }

  if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
    // FIXME: This needs to handle more key events, I think?
    /* key events */
    int down = evt->type == SDL_KEYDOWN;
    const Uint8 *state = SDL_GetKeyboardState(0);
    SDL_Keycode sym = evt->key.keysym.sym;
    if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
      nk_input_key(ctx, NK_KEY_SHIFT, down);
    else if (sym == SDLK_DELETE)
      nk_input_key(ctx, NK_KEY_DEL, down);
    else if (sym == SDLK_RETURN)
      nk_input_key(ctx, NK_KEY_ENTER, down);
    else if (sym == SDLK_TAB)
      nk_input_key(ctx, NK_KEY_TAB, down);
    else if (sym == SDLK_BACKSPACE)
      nk_input_key(ctx, NK_KEY_BACKSPACE, down);
    else if (sym == SDLK_HOME) {
      nk_input_key(ctx, NK_KEY_TEXT_START, down);
      nk_input_key(ctx, NK_KEY_SCROLL_START, down);
    } else if (sym == SDLK_END) {
      nk_input_key(ctx, NK_KEY_TEXT_END, down);
      nk_input_key(ctx, NK_KEY_SCROLL_END, down);
    } else if (sym == SDLK_PAGEDOWN) {
      nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
    } else if (sym == SDLK_PAGEUP) {
      nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
    } else if (sym == SDLK_z)
      nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_r)
      nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_c)
      nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_v)
      nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_x)
      nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_b)
      nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_e)
      nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
    else if (sym == SDLK_UP)
      nk_input_key(ctx, NK_KEY_UP, down);
    else if (sym == SDLK_DOWN)
      nk_input_key(ctx, NK_KEY_DOWN, down);
    else if (sym == SDLK_LEFT) {
      if (state[SDL_SCANCODE_LCTRL])
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
      else
        nk_input_key(ctx, NK_KEY_LEFT, down);
    } else if (sym == SDLK_RIGHT) {
      if (state[SDL_SCANCODE_LCTRL]) {
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
      } else {
        nk_input_key(ctx, NK_KEY_RIGHT, down);
      }
    } else {
      return 0;
    }
    return 1;
  } else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
    /* mouse button */
    int down = evt->type == SDL_MOUSEBUTTONDOWN;
    const int x = evt->button.x;
    const int y = evt->button.y;
    if (evt->button.button == SDL_BUTTON_LEFT) {
      if (evt->button.clicks > 1) {
        nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
      }
      // TODO: Do we even want this? Should this be entirely optional?
      SDL_SetRelativeMouseMode(!down ? SDL_FALSE : SDL_TRUE);  // Hide/show cursor when dragging
      nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
    } else if (evt->button.button == SDL_BUTTON_MIDDLE) {
      nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
    } else if (evt->button.button == SDL_BUTTON_RIGHT) {
      nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
    }
    return 1;
  } else if (evt->type == SDL_MOUSEMOTION) {
    /* mouse motion */
    if (ctx->input.mouse.grabbed) {
      int x = (int)ctx->input.mouse.prev.x;
      int y = (int)ctx->input.mouse.prev.y;
      // FIXME: Can we use this to fix the slider scaling issues?
      nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
    } else {
      // FIXME: Can we use this to fix the slider scaling issues?
      nk_input_motion(ctx, evt->motion.x, evt->motion.y);
    }
    return 1;
  } else if (evt->type == SDL_TEXTINPUT) {
    /* text input */
    nk_glyph glyph;
    memcpy(glyph, evt->text.text, NK_UTF_SIZE);
    nk_input_glyph(ctx, glyph);
    return 1;
  } else if (evt->type == SDL_MOUSEWHEEL) {
    /* mouse wheel */
    nk_input_scroll(ctx, nk_vec2((float)evt->wheel.x, (float)evt->wheel.y));
    return 1;
  }
  return 0;
}

NK_API
void nk_sdl_shutdown(void) {
  nk_free(&sdl.ctx);
  free(sdl.user_font);
  SDL_DestroyTexture(sdl.target);
  nk_buffer_free(&sdl.cmds);
  memset(&sdl, 0, sizeof(sdl));
}

#endif
