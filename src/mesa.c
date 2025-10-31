#include <SDL2/SDL.h>
#include "AUX.h"

#include <assert.h>
#include <inttypes.h>


struct estado_mesa {
    bool init;
} mesa;

void mesa_setup(SDL_Renderer* ren) {
    UNUSED(ren);
    mesa.init = true;
}

enum tela mesa_loop(SDL_Renderer* ren, SDL_Event evt) {
    const int rw = W_WIDTH/10, rh = rw*3/2,
              sep = rw+pad, hmid = (W_HEIGHT-rh)/2;
    static DragDropRect quadrados[] = {
        { .r.x = sep*0, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*1, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*2, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*3, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*4, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*5, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*6, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*7, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*8, .r.y = hmid, .r.w=rw, .r.h=rh },
        { .r.x = sep*9, .r.y = hmid, .r.w=rw, .r.h=rh },
    };
    static size_t clicks[LEN(quadrados)] = {0};

    enum tela prox_tela = MESA;
    switch (evt.type) {
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: prox_tela = MENU; break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_SURECLICKEVENT: {
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

