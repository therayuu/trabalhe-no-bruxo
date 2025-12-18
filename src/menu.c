#include <SDL2/SDL.h>
#include "AUX.h"

#include <assert.h>
#include <inttypes.h>


struct estado_menu {
    bool init;
    uint8_t cursor;
    SDL_Texture* img;
} menu;


void menu_setup(SDL_Renderer* ren) {
    menu.init = true;
    menu.cursor = 0;
    menu.img = IMG_LoadTexture(ren, ASSETS"fundo_menu.png");
    assert(menu.img != NULL);
}

const int w = W_WIDTH/5, h = W_HEIGHT/10, pad = h/10, sep = h + pad;
static AUX_Button botoes[] = {
  {
    {.x=W_WIDTH/2 - w/2, .y=(W_HEIGHT - sep*4)+sep/8 + sep*0, .w=w, .h=h},
    .label = "mesa", .out = MESA, //! trocar de volta pra "jogar"
  },
  {
    {.x=W_WIDTH/2 - w/2, .y=(W_HEIGHT - sep*4)+sep/8 + sep*1, .w=w, .h=h},
    .label = "loja", .out = LOJA, //! trocar de volta pra "config"
  },
  {
    {.x=W_WIDTH/2 - w/2, .y=(W_HEIGHT - sep*4)+sep/8 + sep*2, .w=w, .h=h},
    .label = "sair", .out = ZERO, //!
  },
};

enum tela menu_loop(SDL_Renderer* ren, SDL_Event evt) {
    enum tela prox_tela = MENU;
    switch (evt.type) {
      case SDL_KEYDOWN: /*apertando = evt.key.keysym.sym;*/ break;
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_LEFT:  menu.cursor = 0; break;
          case SDLK_RIGHT: menu.cursor = LEN(botoes)-1; break;

          uint8_t inc = 0; fall; {
              case SDLK_DOWN: inc = 1; goto setas;
              case SDLK_UP:   inc = LEN(botoes) - 1;
              setas: {
                  menu.cursor = menu.cursor + inc;
                  menu.cursor %= LEN(botoes);
              }
          } break;

          case SDLK_SPACE: case SDLK_RETURN: {
              prox_tela = botoes[menu.cursor].out;
          } break;
      } break;

      case SDL_MOUSEBUTTONUP: {
          const SDL_Point p = { evt.button.x, evt.button.y };
          for (size_t i = 0; i < LEN(botoes); i++) {
              const AUX_Button bot = botoes[i];
              if (SDL_PointInRect(&p, &bot.box)) prox_tela = bot.out;
          }
      } break;

      case SDL_MOUSEMOTION: {
          const SDL_Point p = { evt.motion.x, evt.motion.y };
          for (size_t i = 0; i < LEN(botoes); i++) {
              const SDL_Rect* box = &botoes[i].box;
              if (SDL_PointInRect(&p, box)) { menu.cursor = i; break; }
          }
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, BRANCO);
              AUX_RenderBackgroundImage(ren, menu.img);
              for (size_t i = 0; i < LEN(botoes); i++) {
                  SDL_Color fundo, frente;
                  if (i == menu.cursor) { fundo = AZUL; frente = BRANCO; }
                  else                  { fundo = BRANCO; frente = AZUL; }
                  AUX_DrawButton(ren, botoes[i], fundo, frente);
              }
              SDL_RenderPresent(ren);
          } break;
      }
    }

    return prox_tela;
}

void menu_free() {
    SDL_DestroyTexture(menu.img);
    menu.img = NULL;
    menu.init = false;
}
