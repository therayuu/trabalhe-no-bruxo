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
