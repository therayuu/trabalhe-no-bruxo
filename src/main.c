#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <assert.h>
#include "AUX.h"


#define ASSETS "../assets/"

#define W_WIDTH  1080
#define W_HEIGHT 720

#define FPS 60
#define TIMEOUT 1000/FPS

#define trans(prev, curr) (((uint16_t)prev<<8) | ((uint16_t)curr))

enum tela {
    ZERO = 0,
    MENU,
    MESA,
};

struct estado_menu {
    bool init;
    uint8_t cursor;
    SDL_Texture* img;
} menu;

void menu_setup(SDL_Renderer* ren) {
    menu.init = true;
    menu.img = IMG_LoadTexture(ren, ASSETS"fundo_menu.png");
    assert(menu.img != NULL);
}

enum tela menu_loop(SDL_Renderer* ren, SDL_Event evt) {
    switch (evt.type) {
      case SDL_KEYDOWN: switch (evt.key.keysym.sym) {
          case SDLK_UP:   case SDLK_DOWN:  break;
          case SDLK_LEFT: case SDLK_RIGHT: break;
      } break;
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_UP:   case SDLK_DOWN:  break;
          case SDLK_LEFT: case SDLK_RIGHT: break;
      } break;
      case SDL_MOUSEMOTION: {
          // evt.button.x; evt.button.y;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, BRANCO);
              AUX_RenderBackgroundImage(ren, menu.img);
              SDL_RenderPresent(ren);
          } break;
      }
    }

    return MENU; //!
}

void menu_free() {
    SDL_DestroyTexture(menu.img);
    menu.img = NULL;
    menu.init = false;
}

int main() {
    /* INICIALIZACAO */
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(0);
    SDL_Window* win = SDL_CreateWindow("Trabalhe no bruxó",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         W_WIDTH, W_HEIGHT, SDL_WINDOW_SHOWN
                      );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    /* ESTADO */
    enum tela tela = ZERO;

    /* EXECUÇÃO */
    uint32_t falta = TIMEOUT;
    for (SDL_Event evt; evt.type != SDL_QUIT; ) {
        AUX_NextEvent(&evt, &falta, TIMEOUT);

        enum tela prox;
        switch (tela) {
            case ZERO: prox = MENU; break;
            case MENU: prox = menu_loop(ren, evt); break;
            case MESA: break;
        }

        switch (trans(tela, prox)) {
            case trans(ZERO, MENU):
            case trans(MESA, MENU): menu_setup(ren); break;
        }
        tela = prox;
    }

    /* FINALIZAÇÃO */
    menu_free();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
