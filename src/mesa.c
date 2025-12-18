#include <SDL2/SDL.h>
#include "AUX.h"

#include <assert.h>
#include <inttypes.h>

#define COR_ZONA_FUSAO VERDE_FRACO

enum tipo_carta {
    CARTA_NADA = 0,

    CARTA_FOGO,
    CARTA_AGUA,
    CARTA_TERRA,
    CARTA_AR,

    CARTA_VAPOR,
    CARTA_LAMA,
    CARTA_SOM,

    POCAO_LAMA,
    POCAO_SOM,

    NUM_TIPOS_CARTA,
};

char* tipo_carta_str[NUM_TIPOS_CARTA] = {
    [CARTA_NADA]  = "nada",
    [CARTA_FOGO]  = "fogo",
    [CARTA_AGUA]  = "agua",
    [CARTA_TERRA] = "terra",
    [CARTA_AR]    = "ar",
    [CARTA_VAPOR] = "vapor",
    [CARTA_LAMA]  = "lama",
    [CARTA_SOM]   = "som",
    [POCAO_LAMA]  = "pocao_lama",
    [POCAO_SOM]   = "pocao_som",
};

enum tipo_carta tipo_carta_from_str(const char* s) {
    for (size_t i = 0; i < LEN(tipo_carta_str); i++) {
        if (streql(tipo_carta_str[i], s))
            return (enum tipo_carta)i;
    }
    return (enum tipo_carta)0;
}

struct carta {
    DragDropRect drag;
    enum tipo_carta tipo;
    uint16_t cliques;
};

void FillMergeEvent(SDL_Event* evt, struct carta* carta) {
     evt->user = (SDL_UserEvent) {
         .type = SDL_USEREVENT,
         .code = AUX_MERGEEVENT,
         .data1 = (void*)carta->tipo,
         .timestamp = SDL_GetTicks(),
     };
}
void EmitMergeEvent(struct carta* carta) {
    SDL_Event evt; FillMergeEvent(&evt, carta);
    SDL_PushEvent(&evt);
}

static AUX_Texture fundo_carta = { .img_path = ASSETS"fundo_carta.png" };
static AUX_Texture imagens_cartas[NUM_TIPOS_CARTA] = {
    [CARTA_NADA ] = { .color = &TRANSPARENTE },
    [CARTA_AGUA ] = { .img_path = ASSETS"carta_agua.png" },
    [CARTA_FOGO ] = { .img_path = ASSETS"carta_fogo.png" },
    [CARTA_TERRA] = { .img_path = ASSETS"carta_terra.png" },
    [CARTA_AR   ] = { .img_path = ASSETS"carta_ar.png" },
    [CARTA_LAMA ] = { .img_path = ASSETS"carta_lama.png" },
    [CARTA_SOM  ] = { .img_path = ASSETS"carta_som.png" },
    [CARTA_VAPOR] = {
        .color = &CINZA,
        .img_path = ASSETS"carta_vapor.png"
    },

    [POCAO_LAMA] = { .img_path = ASSETS"carta_poção_lama.png" },
    [POCAO_SOM ] = { .img_path = ASSETS"carta_poção_som.png" },
};

void desenhar_carta(SDL_Renderer* ren, const struct carta carta) {
    const SDL_Rect      rect = transmute(SDL_Rect, carta);
    const DragDropRect  drag = transmute(DragDropRect, carta);
    const DragDropState estado = drag.state;

    const bool virada    = (carta.cliques % 2) != 0;
    const bool clicada   = (estado == CLICKING);
    const bool arrastada = (estado == DRAGGING);

    if (virada) { //! desenhar a textura da parte de trás da carta
        AUX_SetRenderDrawColor(ren, MARROM);
        SDL_RenderFillRect(ren, &rect);
        return;
    } else {
        AUX_RenderTexture(ren, fundo_carta, &rect);
        if (clicada) { //! desenhar a carta maior (levantada)
            AUX_SetRenderDrawColor(ren, AZUL);
            SDL_RenderFillRect(ren, &rect);
        } else if (arrastada) { //! desenhar a carta maior (levantada)
            AUX_SetRenderDrawColor(ren, VERMELHO);
            SDL_RenderFillRect(ren, &rect);
        }
    }

    const int l = rect.w/2;
    SDL_Rect quad = {.w=l, .h=l};
    AUX_CenterRect(&quad, rect);

    AUX_Texture tex = imagens_cartas[carta.tipo];
    if (tex.img) AUX_RenderTexture(ren, tex, &rect);
    else         AUX_RenderTexture(ren, tex, &quad);
}

enum tipo_carta combinar(const enum tipo_carta t1, const enum tipo_carta t2) {
    switch (par(t1, t2)) {
        case par(CARTA_FOGO,  CARTA_AGUA): return CARTA_VAPOR;
        case par(CARTA_TERRA, CARTA_AGUA): return CARTA_LAMA;
        case par(CARTA_FOGO,  CARTA_AR):   return CARTA_SOM;

        case par(CARTA_LAMA,  CARTA_NADA): return POCAO_LAMA;
        case par(CARTA_SOM,   CARTA_NADA): return POCAO_SOM;

        default: return CARTA_NADA;
    }
}

struct carta fundir(struct carta curr, struct carta next) {
    curr.tipo = combinar(curr.tipo, next.tipo);
    curr.drag.r.x = (curr.drag.r.x + next.drag.r.x)/2;
    curr.drag.r.y = (curr.drag.r.y + next.drag.r.y)/2;
    return curr;
}

static const int fusao_w = W_WIDTH  / 6;
static const int fusao_h = W_HEIGHT / 4;
static SDL_Rect zona_fusao = {
    .w = fusao_w, .h = fusao_h,
    .x = (W_WIDTH  - fusao_w)/2,
    .y = (W_HEIGHT - fusao_h)/2,
};


struct estado_mesa {
    bool init;
} mesa;

void mesa_setup(SDL_Renderer* ren) {
    mesa.init = true;

    AUX_TextureInit(ren, &fundo_carta);
    for (size_t i = 0; i < LEN(imagens_cartas); i++) {
        AUX_TextureInit(ren, &imagens_cartas[i]);
    }
}

//! claramente fora de lugar
//! deveria ser um ponteiro dentro do struct, que atualiza a partir de um evento
enum tipo_carta pocoes_possiveis[5] = {0}; //! hardcoded

enum tela mesa_loop(SDL_Renderer* ren, SDL_Event evt) {
    const int rw = W_WIDTH/10, rh = rw*3/2, sep = rw+pad,
              hmax = W_HEIGHT-rh, wmax = W_WIDTH-rw;
    static struct carta cartas[] = {
        {
            .drag={ .r.x = sep*0, .r.y = hmax*2/3, .r.w=rw, .r.h=rh }, .tipo=0,
        }, {
            .drag={ .r.x = sep*0, .r.y = hmax*1/3, .r.w=rw, .r.h=rh }, .tipo=0,
        }, {
            .drag={ .r.x = sep*1, .r.y = hmax*2/3, .r.w=rw, .r.h=rh }, .tipo=1,
        }, {
            .drag={ .r.x = sep*1, .r.y = hmax*1/3, .r.w=rw, .r.h=rh }, .tipo=1,
        }, {
            .drag={ .r.x = sep*2, .r.y = hmax*2/3, .r.w=rw, .r.h=rh }, .tipo=2,
        }, {
            .drag={ .r.x = sep*2, .r.y = hmax*1/3, .r.w=rw, .r.h=rh }, .tipo=2,
        },
        {
            .drag={ .r.x = wmax - sep*2, .r.y = hmax*2/3, .r.w=rw, .r.h=rh }, .tipo=3,
        }, {
            .drag={ .r.x = wmax - sep*2, .r.y = hmax*1/3, .r.w=rw, .r.h=rh }, .tipo=3,
        }, {
            .drag={ .r.x = wmax - sep*1, .r.y = hmax*2/3, .r.w=rw, .r.h=rh }, .tipo=4,
        }, {
            .drag={ .r.x = wmax - sep*1, .r.y = hmax*1/3, .r.w=rw, .r.h=rh }, .tipo=4,
        }, {
            .drag={ .r.x = wmax - sep*0, .r.y = hmax*2/3, .r.w=rw, .r.h=rh }, .tipo=0,
        }, {
            .drag={ .r.x = wmax - sep*0, .r.y = hmax*1/3, .r.w=rw, .r.h=rh }, .tipo=0,
        },
    };
    static size_t num_cartas = LEN(cartas);

    enum tela prox_tela = MESA;
    switch (evt.type) {
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: prox_tela = MENU; break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_SURECLICKEVENT: {
              cartas[num_cartas-1].cliques += 1;
          } break;

          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, BRANCO);
              AUX_SetRenderDrawColor(ren, COR_ZONA_FUSAO);
              SDL_RenderFillRect(ren, &zona_fusao);

              for (size_t i = 0; i < num_cartas; i++) {
                  desenhar_carta(ren, cartas[i]);
              }

              SDL_RenderPresent(ren);
          } break;
      }
    }

    for (size_t i = num_cartas; i--; ) {
        AUX_DragDropCancel(&cartas[i].drag, evt);
        if (cartas[i].drag.state != UNCLICKED) {
            AUX_ToEndLen(cartas, num_cartas, i); break;
        }
    }

    struct carta* last = &cartas[num_cartas-1];
    if (last->drag.state == UNCLICKED && SDL_HasIntersection(&last->drag.r, &zona_fusao)) {
        for (size_t i = num_cartas-1; i--;) {
            const struct carta* curr = &cartas[i];

            if (SDL_HasIntersection(&last->drag.r, &curr->drag.r) &&
                SDL_HasIntersection(&curr->drag.r, &zona_fusao)) {
                struct carta n = fundir(*last, *curr);
                if (n.tipo != CARTA_NADA) {
                    *last = n; AUX_RemoveUnordered(cartas, num_cartas, i);
                    if (AUX_NullTerminatedFind(pocoes_possiveis, &n.tipo)) {
                        EmitMergeEvent(&n); return LOJA;
                    }
                    break;
                }
            }
        }
    }

    return prox_tela;
}

void mesa_free() {
    mesa.init = false;
}

