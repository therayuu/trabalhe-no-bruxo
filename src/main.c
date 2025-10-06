#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <assert.h>

#include "AUX.h"
#include "comum.h"
#include "menu.c"


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
            case trans(MENU, MESA):
            case trans(MENU, ZERO): menu_free(ren); break;
        }
        tela = prox;
    }

    /* FINALIZAÇÃO */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
