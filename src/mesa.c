#include <SDL2/SDL.h>
#include "AUX.h"

#include <assert.h>
#include <inttypes.h>


enum tipo_carta {
    CARTA_NADA = 0,

    CARTA_FOGO,
    CARTA_AGUA,
    CARTA_TERRA,
    CARTA_AR,

    NUM_TIPOS_CARTA,
};

struct carta {
    DragDropRect rect;
    enum tipo_carta tipo;
    uint16_t cliques;
};

static AUX_Texture fundo_carta = { .color = &PRETO };
static AUX_Texture imagens_cartas[NUM_TIPOS_CARTA] = {
    [CARTA_NADA ] = { .color = NULL },
    [CARTA_AGUA ] = { .color = &AZUL },
    [CARTA_FOGO ] = { .color = &LARANJA },
    [CARTA_TERRA] = { .color = &MARROM },
    [CARTA_AR   ] = { .color = &BRANCO },
};
void desenhar_carta(SDL_Renderer* ren, const struct carta carta) {
    const SDL_Rect      rect = transmute(SDL_Rect, carta);
    const DragDropRect  drag = transmute(DragDropRect, carta);
    const DragDropState estado = drag.state;

    const bool virada    = (carta.cliques % 2) != 0;
    const bool clicada   = (estado == CLICKING);
    const bool arrastada = (estado == DRAGGING);

    if (clicada) { //! desenhar a carta selecionada/maior
        AUX_SetRenderDrawColor(ren, AZUL);
        SDL_RenderFillRect(ren, &rect);
    } else if (arrastada) { //! desenhar a carta selecionada/maior
        AUX_SetRenderDrawColor(ren, VERMELHO);
        SDL_RenderFillRect(ren, &rect);
    } else if (virada) { //! desenhar a parte de trás da carta
        AUX_SetRenderDrawColor(ren, CINZA);
        SDL_RenderFillRect(ren, &rect);
    } else {
        AUX_RenderTexture(ren, fundo_carta, &rect);
    }

    SDL_Rect img = { .w = rect.w/2, .h = rect.w/2 };
    AUX_CenterRect(&img, rect);
    AUX_RenderTexture(ren, imagens_cartas[carta.tipo], &img);
}


struct estado_mesa {
    bool init;
} mesa;

void mesa_setup(SDL_Renderer* ren) {
    UNUSED(ren);
    mesa.init = true;

    for (size_t i = 0; i < LEN(imagens_cartas); i++) {
        if (!imagens_cartas[i].color)
            imagens_cartas[i].color = &CINZA;
    }
    imagens_cartas[CARTA_NADA].color = NULL;
}

enum tela mesa_loop(SDL_Renderer* ren, SDL_Event evt) {
    const int rw = W_WIDTH/10, rh = rw*3/2,
              sep = rw+pad, hmid = (W_HEIGHT-rh)/2;
    static struct carta cartas[] = {
        {
            .rect = { .r.x = sep*0, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 0, .cliques = 0,
        }, {
            .rect = { .r.x = sep*1, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 1, .cliques = 0,
        }, {
            .rect = { .r.x = sep*2, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 2, .cliques = 0,
        }, {
            .rect = { .r.x = sep*3, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 3, .cliques = 0,
        }, {
            .rect = { .r.x = sep*4, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 4, .cliques = 0,
        }, {
            .rect = { .r.x = sep*5, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 0, .cliques = 0,
        }, {
            .rect = { .r.x = sep*6, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 1, .cliques = 0,
        }, {
            .rect = { .r.x = sep*7, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 2, .cliques = 0,
        }, {
            .rect = { .r.x = sep*8, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 3, .cliques = 0,
        }, {
            .rect = { .r.x = sep*9, .r.y = hmid, .r.w=rw, .r.h=rh },
            .tipo = 4, .cliques = 0,
        },
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
                  desenhar_carta(ren, carta);
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

