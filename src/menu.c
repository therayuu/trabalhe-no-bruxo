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

const int w = W_WIDTH/5, h = W_HEIGHT/10, pad = h/10;
static SDL_Rect botoes[] = {
    { .x=W_WIDTH/2 - w/2, .y=W_HEIGHT/2 + (h+pad)*0, .w=w, .h=h },
    { .x=W_WIDTH/2 - w/2, .y=W_HEIGHT/2 + (h+pad)*1, .w=w, .h=h },
    { .x=W_WIDTH/2 - w/2, .y=W_HEIGHT/2 + (h+pad)*2, .w=w, .h=h },
};

enum tela tela_do_botao(const SDL_Rect* bot) {
    enum { JOGAR=0, CONF, SAIR };
    if (bot == &botoes[JOGAR]) return MESA;
    if (bot == &botoes[CONF])  return MENU; //!
    if (bot == &botoes[SAIR])  return MENU; //!
    return MENU;
}

enum tela menu_loop(SDL_Renderer* ren, SDL_Event evt) {
    enum tela prox_tela = MENU;
    switch (evt.type) {
      case SDL_KEYDOWN: /*apertando = evt.key.keysym.sym;*/ break;
      case SDL_KEYUP: switch (evt.key.keysym.sym) {
          case SDLK_LEFT:  menu.cursor = 0; break;
          case SDLK_RIGHT: menu.cursor = LEN(botoes)-1; break;

          uint8_t inc = 0; {
              case SDLK_DOWN: inc = 1; goto setas;
              case SDLK_UP:   inc = LEN(botoes) - 1;
              setas: {
                  menu.cursor = menu.cursor + inc;
                  menu.cursor %= LEN(botoes);
              }
          } break;

          case SDLK_RETURN: {
              prox_tela = tela_do_botao(&botoes[menu.cursor]);
          } break;
      } break;

      case SDL_MOUSEBUTTONUP: {
          const SDL_Rect r = { evt.button.x, evt.button.y, 1,1 };
          for (size_t i = 0; i < LEN(botoes); i++) {
              const SDL_Rect* bot = &botoes[i];
              if (SDL_HasIntersection(bot, &r))
                  prox_tela = tela_do_botao(bot);
          }
      } break;

      case SDL_MOUSEMOTION: {
          const SDL_Rect r = { evt.motion.x, evt.motion.y, 1,1 };
          for (size_t i = 0; i < LEN(botoes); i++) {
              const SDL_Rect* bot = &botoes[i];
              if (SDL_HasIntersection(bot, &r)) {
                  menu.cursor = i; break;
              }
          }
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, BRANCO);
              AUX_RenderBackgroundImage(ren, menu.img);

              for (size_t i = 0; i < LEN(botoes); i++) {
                  const SDL_Rect* bot = &botoes[i];
                  if (i == menu.cursor) AUX_SetRenderDrawColor(ren, AZUL);
                  else                  AUX_SetRenderDrawColor(ren, BRANCO);
                  SDL_RenderDrawRect(ren, bot);
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
