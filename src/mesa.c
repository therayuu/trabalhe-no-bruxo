struct estado_mesa {
    bool init;
} mesa;

void mesa_setup(SDL_Renderer* ren) {
    mesa.init = true;
}

enum tela mesa_loop(SDL_Renderer* ren, SDL_Event evt) {
    enum tela prox_tela = MESA;
    switch (evt.type) {
      case SDL_KEYDOWN: break;
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: prox_tela = MENU; break;
      } break;

      case SDL_MOUSEBUTTONUP: {
          const SDL_Rect r = { evt.button.x, evt.button.y, 1,1 };
      } break;

      case SDL_MOUSEMOTION: {
          const SDL_Rect r = { evt.motion.x, evt.motion.y, 1,1 };
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, VERDE);
              SDL_RenderPresent(ren);
          } break;
      }
    }

    return prox_tela;
}

void mesa_free() {
    mesa.init = false;
}
