#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <assert.h>
#include "AUX.h"


#define ASSETS_DIR "img/"

#define W_WIDTH  1080
#define W_HEIGHT 720

#define FPS 60
#define TIMEOUT 1000/FPS


int main() {
    /* INICIALIZACAO */
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(0);
    SDL_Window* win = SDL_CreateWindow("Imagem",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         W_WIDTH, W_HEIGHT, SDL_WINDOW_SHOWN
                      );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
    SDL_Texture* img = IMG_LoadTexture(ren, ASSETS_DIR"tela_inicial.png");
    assert(img != NULL);

    /* EXECUÇÃO */
    uint32_t falta = TIMEOUT;
    for (SDL_Event evt; evt.type != SDL_QUIT; ) {
        if (AUX_WaitEventTimeout(&evt, &falta, TIMEOUT)) {
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
            }
        } else {
            AUX_RenderClearColor(ren, BRANCO);
            AUX_RenderBackgroundImage(ren, img);
            SDL_RenderPresent(ren);
        }
    }

    /* FINALIZACAO */
    SDL_DestroyTexture(img);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
