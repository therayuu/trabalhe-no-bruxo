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

struct carta {
    DragDropRect rect;
    uint16_t cliques;

    //! SDL_Texture* img;
    //! SDL_Color* color;
};

enum tela mesa_loop(SDL_Renderer* ren, SDL_Event evt) {
    const int rw = W_WIDTH/10, rh = rw*3/2,
              sep = rw+pad, hmid = (W_HEIGHT-rh)/2;
    static struct carta cartas[] = {
        {{ .r.x = sep*0, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*1, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*2, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*3, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*4, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*5, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*6, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*7, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*8, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
        {{ .r.x = sep*9, .r.y = hmid, .r.w=rw, .r.h=rh }, .cliques = 0},
    };

    enum tela prox_tela = MESA;
    switch (evt.type) {
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: prox_tela = MENU; break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_SURECLICKEVENT: {
              cartas[LEN(cartas)-1].cliques += 1;
          } break;

          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, BRANCO);
              for (size_t i = 0; i < LEN(cartas); i++) {
                  const struct carta carta = cartas[i];
                  const int estado = transmute(DragDropRect, carta).state;
                  const SDL_Color cor = (estado == CLICKING) ?
                      VERMELHO : (carta.cliques % 2) ? AZUL : PRETO
                  ;
                  AUX_SetRenderDrawColor(ren, cor);
                  SDL_RenderFillRect(ren, (SDL_Rect*)&carta);
                  //! if(carta.img) SDL_RenderCopy(ren, carta.img, NULL, &carta);
              }

              SDL_RenderPresent(ren);
          } break;
      }
    }

    for (size_t i = LEN(cartas); i--; ) {
        AUX_DragDropCancel(&cartas[i].rect, evt);
        if (cartas[i].rect.state != UNCLICKED) {
            AUX_ToEnd(cartas, i); break;
        }
    }
    return prox_tela;
}

void mesa_free() {
    mesa.init = false;
}

