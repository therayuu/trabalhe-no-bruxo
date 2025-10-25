#include <assert.h>

#define asClick(evt) transmute(SDL_MouseButtonEvent, evt)

static inline
void AUX_ToEndSzLen(void* arr, size_t size, size_t len, size_t idx);
#define AUX_ToEndLen(arr, len, i) AUX_ToEndSzLen(arr, sizeof(*arr), len, i)
#define AUX_ToEnd(arr, i) AUX_ToEndLen(arr, LEN(arr), i)

typedef struct {
    SDL_Rect r;

    SDL_MouseButtonEvent click;
    SDL_Point offset;
    enum {
        UNCLICKED = 0,
        CLICKING,
        DRAGGING,

        DRAG_STATE_COUNT,
    } state;
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
}

struct estado_mesa {
    bool init;
} mesa;

void mesa_setup(SDL_Renderer* ren) {
    UNUSED(ren);
    mesa.init = true;
}

const int rw = W_WIDTH/10, hmid = (W_HEIGHT-rw)/2;
static DragDropRect quadrados[4] = {
  { .r.x = W_WIDTH*1/3-rw/2, .r.y = hmid,    .r.w=rw, .r.h=rw },
  { .r.x = W_WIDTH*2/3-rw/2, .r.y = hmid,    .r.w=rw, .r.h=rw },
  { .r.x = W_WIDTH/2  -rw/2, .r.y = hmid-rw, .r.w=rw, .r.h=rw },
  { .r.x = W_WIDTH/2  -rw/2, .r.y = hmid+rw, .r.w=rw, .r.h=rw },
};
static size_t clicks[4] = {0};

enum tela mesa_loop(SDL_Renderer* ren, SDL_Event evt) {
    enum tela prox_tela = MESA;
    switch (evt.type) {
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: prox_tela = MENU; break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_SURECLICKEVENT: {
              // size_t idx = evt.user.data1 - (void*)quadrados;
              clicks[LEN(clicks)-1] += 1;
          } break;

          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, BRANCO);
              for (size_t i = 0; i < LEN(quadrados); i++) {
                  const SDL_Color idle[] = {AZUL_BEBE, CINZA, AMARELO_M};

                  const SDL_Color cor[DRAG_STATE_COUNT] = {
                      clicks[i] ? idle[(clicks[i]-1) % LEN(idle)]
                                : PRETO,
                                  AZUL, VERMELHO_M
                  };

                  const uint8_t idx = quadrados[i].state;
                  AUX_SetRenderDrawColor(ren, cor[idx]);
                  SDL_RenderFillRect(ren, &quadrados[i].r);
              }

              SDL_RenderPresent(ren);
          } break;
      }
    }

    for (size_t i = LEN(quadrados); i--; ) {
        AUX_DragDropCancel(&quadrados[i], evt);
        if (quadrados[i].state != UNCLICKED) {
            AUX_ToEnd(quadrados, i); AUX_ToEnd(clicks, i); break;
        }
    }
    return prox_tela;
}

void mesa_free() {
    mesa.init = false;
}

static inline
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

