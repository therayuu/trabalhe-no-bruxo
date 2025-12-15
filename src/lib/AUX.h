#ifndef _AUX_H_
#define _AUX_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <ctype.h>
#include "cores.h"


/* MACROS */
#define LEN(arr) (sizeof(arr)/sizeof(*arr))
#define UNUSED(x) (void) x
#define UNREACHABLE() assert(false && "unreachable")

#define as(T) *(T*)&
#define transmute(T, x) (as(T)x)

#define fall [[fallthrough]]


/* EVENTOS */
#define DT(...) AUX_dt(__VA_ARGS__)
int32_t AUX_dt(uint32_t antes, uint32_t* depois) {
    uint32_t agora = SDL_GetTicks();
    uint32_t delta = agora - antes;

    if (depois) *depois = agora;
    return delta;
}

typedef enum {
    AUX_TIMEOUTEVENT = 0,
    AUX_SURECLICKEVENT,
    AUX_FIRSTUSEREVENT,
} AUX_EventType;

bool AUX_WaitEventTimeoutCount(SDL_Event* evt, uint32_t* ms) {
    static uint32_t antes;
    if (antes == 0) antes = SDL_GetTicks();

    bool evento = SDL_WaitEventTimeout(evt, *ms);
    if (evento) {
        uint32_t delta = DT(antes, &antes);
        *ms = (delta < *ms) ? *ms - delta : 0;
    }
    return evento;
}

bool AUX_WaitEventTimeout(SDL_Event* evt, uint32_t* ms, uint32_t timeout) {
    bool evento = AUX_WaitEventTimeoutCount(evt, ms);
    if (!evento) *ms = timeout;

    return evento;
}

void AUX_FillTimeout(SDL_Event* evt) {
     evt->user = (SDL_UserEvent) {
         .type = SDL_USEREVENT,
         .code = AUX_TIMEOUTEVENT,
         .timestamp = SDL_GetTicks(),
     };
}

void AUX_NextEvent(SDL_Event* evt, uint32_t* falta, uint32_t timeout) {
    if (AUX_WaitEventTimeout(evt, falta, timeout));
    else AUX_FillTimeout(evt);
}


/* MATEMÁTICA */
void AUX_ClampRectPos(SDL_Rect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x;
    if (ret->y < win.y) ret->y = win.y;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x+win.w - ret->w;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y+win.h - ret->h;
}

void AUX_ClampRectPosF(SDL_FRect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x;
    if (ret->y < win.y) ret->y = win.y;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x+win.w - ret->w;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y+win.h - ret->h;
}

void AUX_WrapRectPos(SDL_Rect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x+win.w - ret->w;
    if (ret->y < win.y) ret->y = win.y+win.h - ret->h;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y;
}

void AUX_WrapRectPosF(SDL_FRect* ret, const SDL_Rect win) {
    if (ret->x < win.x) ret->x = win.x+win.w - ret->w;
    if (ret->y < win.y) ret->y = win.y+win.h - ret->h;

    if (ret->x+ret->w > win.x+win.w) ret->x = win.x;
    if (ret->y+ret->h > win.y+win.h) ret->y = win.y;
}

void AUX_CenterRect(SDL_Rect* inner, SDL_Rect outer) {
    SDL_Rect r = *inner;
    SDL_Point c = {
        .x = outer.x + outer.w/2,
        .y = outer.y + outer.h/2,
    };
    inner->x = c.x - r.w/2;
    inner->y = c.y - r.h/2;
}

/* MISCELÂNEA */
typedef struct string_view {
    const char* buf;
    size_t len;
} AUX_StringView;

#define AUX_SV(str) ((AUX_StringView){str, strlen(str)})
#define AUX_SVToCstr(str) (strncpy(calloc(str.len, 1), str.buf, str.len))

#define AUX_StringViewEq(a, b) (AUX_StringViewCmp(a, b) == 0)
int AUX_StringViewCmp(AUX_StringView a, AUX_StringView b) {
    return strncmp(a.buf, b.buf, a.len);
}

static inline
#define AUX_ToEnd(arr, i) AUX_ToEndLen(arr, LEN(arr), i)
#define AUX_ToEndLen(arr, len, i) AUX_ToEndSzLen(arr, sizeof(*arr), len, i)
void AUX_ToEndSzLen(void* arr, size_t size, size_t len, size_t idx) {
  #define elem(arr, size, idx) ((arr) + (size)*(idx))
    char *const base = arr;
    char *const curr = elem(base, size, idx);

    char buf[size];
    memcpy(buf, curr, size);

    char *const next = elem(base, size, idx+1);
    char *const last = elem(base, size, len-1);

    memmove(curr, next, last-curr);
    memcpy(last, buf, size);
  #undef elem
}

#define AUX_RemoveUnordered(arr, len, i) do { \
    AUX_ToEndLen(arr, len, i); len -= 1; \
} while(0)


/* GRÁFICOS */
typedef struct {
    SDL_Texture* img;
    const char* img_path;
    const SDL_Color* color;
} AUX_Texture;

SDL_Color AUX_GetRenderDrawColor(SDL_Renderer *renderer) {
    SDL_Color cor;
    SDL_GetRenderDrawColor(renderer, splat(&cor));
    return cor;
}

void AUX_SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color cor) {
    SDL_SetRenderDrawColor(renderer, splat(cor));
}

void AUX_RenderClearColor(SDL_Renderer* renderer, SDL_Color cor) {
    AUX_SetRenderDrawColor(renderer, cor);
    SDL_RenderClear(renderer);
}

void AUX_RenderBackgroundImage(SDL_Renderer* renderer, SDL_Texture* img) {
    //! cortar em vez de amassar a imagem
    // SDL_Rect janela = {0};
    // SDL_GetRendererOutputSize(renderer, &janela.w, &janela.h);
    // //! contas aqui
    // SDL_RenderCopy(renderer, img, NULL, &janela);
    SDL_RenderCopy(renderer, img, NULL, NULL);
}

void AUX_TextureInit(SDL_Renderer* ren, AUX_Texture* tex) {
    if (!tex->img && tex->img_path)
        tex->img = IMG_LoadTexture(ren, tex->img_path);
}

bool AUX_RenderTextureTry(SDL_Renderer* ren, const AUX_Texture tex,
                                             const SDL_Rect* const rect) {
    if (tex.img) {
        SDL_RenderCopy(ren, tex.img, NULL, rect);
    } else if (tex.color) {
        AUX_SetRenderDrawColor(ren, *tex.color);
        SDL_RenderFillRect(ren, rect);
    } else return false;

    return true;
}

void AUX_RenderTexture(SDL_Renderer* ren, const AUX_Texture tex,
                                          const SDL_Rect* const rect) {
    if (!AUX_RenderTextureTry(ren, tex, rect)) { //! desenhar xadrez roxo
        AUX_SetRenderDrawColor(ren, MAGENTA);
        SDL_RenderFillRect(ren, rect);
    }
}

/* TEXTO */
#define AUX_DrawTextTTF(ren, font, text, x, y) AUX_DrawTextTTFWrap(ren, font, text, x, y, 0)
void AUX_DrawTextTTFWrap(SDL_Renderer* ren, TTF_Font* font, const char* text,
                         int x, int y, uint32_t wrap) {
    SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(font, text, BRANCO, wrap);
    SDL_Texture* tex  = SDL_CreateTextureFromSurface(ren, surf);

    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, NULL, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void AUX_DrawRectAt(SDL_Renderer* ren,
                    SDL_Rect ret, const int x, const int y) {
    ret.x += x; ret.y += y;
    SDL_RenderFillRect(ren, &ret);
}

void AUX_DrawRectsAt(SDL_Renderer* ren,
                     const struct SDL_Rect* rects, const size_t len,
                     const int x, const int y) {
    for (size_t i = 0; i<len; i++) {
        AUX_DrawRectAt(ren, rects[i], x, y);
    }
}

SDL_Rect AUX_MeasureTextRects(const char* text, int font_sz) {
    return (SDL_Rect){
        .w = font_sz*(strlen(text)+1), //! isso aqui parece muito errado
        .h = font_sz,
    };
}

void AUX_DrawTextRects(SDL_Renderer* ren,
                       const char* texto, const int tam_fonte,
                       const int x, const int y) {
    const int sz  = tam_fonte;
    const int ln  = tam_fonte/8;
    const int pad = tam_fonte/4;

    const int end = sz - ln;
    const int mid = end/2;

    #define HORI sz, ln
    #define VERT ln, sz
      const SDL_Rect TOP_H = { 0,   0, HORI };
      const SDL_Rect BOT_H = { 0, end, HORI };
      const SDL_Rect MID_H = { 0, mid, HORI };

      const SDL_Rect ESQ_V = {   0, 0, VERT };
      const SDL_Rect DIR_V = { end, 0, VERT };
      const SDL_Rect MID_V = { mid, 0, VERT };
    #undef VERT
    #undef HORI

    #define MEIO_HORI sz/2, ln
    #define MEIO_VERT ln, sz/2
      const SDL_Rect MEIO_H_MID_ESQ = {   0, mid, MEIO_HORI };
      const SDL_Rect MEIO_H_MID_DIR = { mid, mid, MEIO_HORI };

      const SDL_Rect MEIO_H_TOP_ESQ = {   0, 0, MEIO_HORI };
      const SDL_Rect MEIO_H_TOP_DIR = { mid, 0, MEIO_HORI };

      const SDL_Rect MEIO_H_BOT_ESQ = {   0, end, MEIO_HORI };
      const SDL_Rect MEIO_H_BOT_DIR = { mid, end, MEIO_HORI };

      const SDL_Rect MEIO_V_TOP_ESQ = {   0, 0, MEIO_VERT };
      const SDL_Rect MEIO_V_TOP_MID = { mid, 0, MEIO_VERT };
      const SDL_Rect MEIO_V_TOP_DIR = { end, 0, MEIO_VERT };

      //const SDL_Rect MEIO_V_BOT_ESQ = {   0, end, MEIO_VERT };
      const SDL_Rect MEIO_V_BOT_MID = { mid, mid, MEIO_VERT };
      const SDL_Rect MEIO_V_BOT_DIR = { end, mid, MEIO_VERT };
    #undef MEIO_VERT
    #undef MEIO_HORI

    const struct SDL_Rect FONTE[][6] = {
        ['a'] = { TOP_H, MID_H,         ESQ_V,        DIR_V, },

        ['c'] = { TOP_H,        BOT_H,  ESQ_V,               },
        ['d'] = { TOP_H,        BOT_H,         MID_V, DIR_V, },
        ['e'] = { TOP_H, MID_H, BOT_H,  ESQ_V,               },
        ['f'] = { TOP_H, MID_H,         ESQ_V,               },

        ['h'] = {        MID_H,         ESQ_V,        DIR_V, },
        ['i'] = { TOP_H,        BOT_H,         MID_V,        },
        ['j'] = {               BOT_H,  DIR_V,               },

        ['l'] = {               BOT_H,  ESQ_V,               },
        ['m'] = { TOP_H,                ESQ_V, MID_V, DIR_V, },
        ['n'] = { TOP_H,                ESQ_V,        DIR_V, },
        ['o'] = { TOP_H,        BOT_H,  ESQ_V,        DIR_V, },

        ['t'] = { TOP_H,                       MID_V,        },
        ['u'] = {               BOT_H,  ESQ_V,        DIR_V, },

        ['w'] = {               BOT_H,  ESQ_V, MID_V, DIR_V, },
        ['x'] = {        MID_H,                MID_V,        }, //!

        ['b'] = {
            MID_H, BOT_H,  ESQ_V,
            MEIO_V_BOT_DIR,
        },
        ['g'] = {
            TOP_H, BOT_H,  ESQ_V,
            MEIO_H_MID_DIR, MEIO_V_BOT_DIR,
        },
        ['k'] = {
            ESQ_V,
            MEIO_H_BOT_DIR, MEIO_H_TOP_DIR,
            MEIO_H_MID_ESQ,
            MEIO_V_TOP_MID, MEIO_V_BOT_MID,
        },
        ['p'] = { TOP_H, MID_H,  ESQ_V,         MEIO_V_TOP_DIR, },
        ['q'] = { TOP_H, BOT_H,  ESQ_V, DIR_V,  MEIO_V_BOT_MID, },
        ['r'] = { MID_V,  MEIO_H_TOP_DIR, },
        ['s'] = { MID_V,  MEIO_H_BOT_ESQ, MEIO_H_TOP_DIR, },
        ['v'] = {
            ESQ_V,
            MEIO_V_BOT_MID, MEIO_V_TOP_DIR,
            MEIO_H_MID_DIR, MEIO_H_BOT_ESQ,
        },
        ['y'] = { MID_H, BOT_H, DIR_V,  MEIO_V_TOP_ESQ,   },
        ['z'] = { MID_V,  MEIO_H_BOT_DIR, MEIO_H_TOP_ESQ, },
    };

    for (size_t i = 0; texto[i]; i++) {
        const size_t c = tolower(texto[i]);
        AUX_DrawRectsAt(ren, FONTE[c], LEN(FONTE[c]),
                             x+(tam_fonte+pad)*i, y);
    }
}

/* OBJETOS */

/** BOTÃO **/
typedef struct {
    SDL_Rect box;
    char* label;
    union {
        intptr_t id, out;
        void* ptr;
    };
} AUX_Button;

void AUX_DrawButton(SDL_Renderer* ren, AUX_Button bot,
                                       SDL_Color fundo,
                                       SDL_Color frente) {
    const SDL_Rect box = bot.box;
    const char* label = bot.label;
    const int tam = box.h*2/6;

    AUX_SetRenderDrawColor(ren, fundo);
    SDL_RenderFillRect(ren, &box);
    AUX_SetRenderDrawColor(ren, frente);

    SDL_Rect text_box = AUX_MeasureTextRects(label, tam);
                        AUX_CenterRect(&text_box, box);
    AUX_DrawTextRects(ren, label, tam, text_box.x, text_box.y);
}

/** RETÂNGULO ARRASTÁVEL **/
typedef enum drag_drop_state {
    UNCLICKED = 0,
    CLICKING,
    DRAGGING,

    DRAG_STATE_COUNT,
} DragDropState;

typedef struct drag_drop_rect {
    SDL_Rect r;

    DragDropState state;
    SDL_MouseButtonEvent click;
    SDL_Point offset;
} DragDropRect;

void AUX__FillSureClick(SDL_Event* evt, DragDropRect* rect) {
     evt->user = (SDL_UserEvent) {
         .type = SDL_USEREVENT,
         .code = AUX_SURECLICKEVENT,
         .data1 = rect,
         .timestamp = SDL_GetTicks(),
     };
}
void AUX__EmitSureClick(DragDropRect* rect) {
    SDL_Event evt; AUX__FillSureClick(&evt, rect);
    SDL_PushEvent(&evt);
}

void AUX_DragDropCancel(DragDropRect* self, SDL_Event evt) {
  #define asClick(evt) transmute(SDL_MouseButtonEvent, evt)
    const bool clicked = (self->state == CLICKING) ||
                         (self->state == DRAGGING);

    switch (evt.type) {
      case SDL_KEYDOWN: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: if (clicked) {
              self->r.x = self->click.x + self->offset.x;
              self->r.y = self->click.y + self->offset.y;

              self->state = UNCLICKED;
          } break;
      } break;
      case SDL_MOUSEBUTTONDOWN: {
          SDL_Point loc = { evt.button.x, evt.button.y };
          if (SDL_PointInRect(&loc, &self->r)) {
              self->offset.x = self->r.x - loc.x;
              self->offset.y = self->r.y - loc.y;
              self->click = asClick(evt);

              self->state = CLICKING;
          }
      } break;
      case SDL_MOUSEBUTTONUP: {
          if (self->state == CLICKING) AUX__EmitSureClick(self);

          self->state = UNCLICKED;
      } break;
      case SDL_MOUSEMOTION: if (clicked) {
          self->r.x = evt.button.x + self->offset.x;
          self->r.y = evt.button.y + self->offset.y;

          self->state = DRAGGING;
      } break;
    }
  #undef asClick
}

#endif//_AUX_H_
