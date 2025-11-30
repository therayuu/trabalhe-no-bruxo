#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define TAM_FONTE 20

typedef struct dialog_option {
    char* text;
    size_t nextNode;
} DialogueOption;

typedef struct dialog_node {
    char* speaker;
    char* text;
    size_t num_opts;
    DialogueOption options[5]; //! tamanho hardcoded
} DialogueNode;

TTF_Font* font;
static struct estado_dialogo {
    size_t num_nodes;
    DialogueNode nodes[300]; //!
    bool waiting_for_choice; //! esse campo não deveria existir
    size_t current_node;
    size_t selected_option;
} dialogo;

void loadDialogue() {
    // Exemplo simples de estrutura de diálogo
    DialogueNode n1 = {
        .speaker = "Alice",
        .text = "Oi! Voce"/*"ê"*/" quer ir ao parque hoje?",
        .num_opts = 0,
    };
    n1.options[n1.num_opts++] = (DialogueOption){"Sim, vamos!", 1};
    n1.options[n1.num_opts++] = (DialogueOption){"Na"/*"ã"*/"o, estou cansado.", 2};

    DialogueNode n2 = {
        .speaker = "Alice",
        .text = /*"Ó"*/"Otimo! O dia esta"/*"á"*/" lindo la"/*"á"*/" fora.",
    };

    DialogueNode n3 = {
        .speaker = "Alice",
        .text = "Tudo bem, talvez outro dia.",
    };

    dialogo.nodes[dialogo.num_nodes++] = n1;
    dialogo.nodes[dialogo.num_nodes++] = n2;
    dialogo.nodes[dialogo.num_nodes++] = n3;
}

void renderText(SDL_Renderer* ren, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, white);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void renderDialog(SDL_Renderer* ren, TTF_Font* font) {
    const DialogueNode node = dialogo.nodes[dialogo.current_node];

    SDL_Rect nameBox = {TAM_FONTE*2, W_WIDTH/2, 10*TAM_FONTE, TAM_FONTE*2 + TAM_FONTE/2};
    SDL_Rect textBox = {TAM_FONTE*2, nameBox.y + nameBox.h, W_WIDTH - textBox.x*2, TAM_FONTE*6};

    SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
    SDL_RenderFillRect(ren, &textBox);

    SDL_SetRenderDrawColor(ren, 80, 80, 80, 255);
    SDL_RenderFillRect(ren, &nameBox);

    renderText(ren, font, node.speaker, nameBox.x + 10, nameBox.y + 10);
    renderText(ren, font, node.text,    textBox.x + 10, textBox.y + 10);

    if (dialogo.waiting_for_choice) {
        for (size_t i = 0; i < node.num_opts; ++i) {
            char* prefix = (i == dialogo.selected_option ? "> " : "  ");
            renderText(ren, font, prefix,
                       textBox.x + TAM_FONTE,
                       textBox.y + TAM_FONTE*3 + i*(TAM_FONTE+5));
            renderText(ren, font, node.options[i].text,
                       textBox.x + TAM_FONTE + TAM_FONTE,
                       textBox.y + TAM_FONTE*3 + i*(TAM_FONTE+5));
        }
    }
}

void dialogo_setup(SDL_Renderer* ren) {
    UNUSED(ren);

    dialogo.num_nodes = 0,
    dialogo.waiting_for_choice = false,
    dialogo.current_node = 0,
    dialogo.selected_option = 0,

    TTF_Init();
    font = TTF_OpenFont(ASSETS"tiny.ttf", TAM_FONTE),
    loadDialogue();
}

enum tela dialogo_loop(SDL_Renderer* ren, SDL_Event evt) {
    const DialogueNode node = dialogo.nodes[dialogo.current_node];
    switch (evt.type) {
      case SDL_KEYDOWN: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: return MENU; //!

          case SDLK_SPACE: case SDLK_RETURN: {
              if (dialogo.waiting_for_choice) {
                  if (dialogo.selected_option < node.num_opts) {
                      dialogo.current_node = node.options[dialogo.selected_option].nextNode;
                      dialogo.waiting_for_choice = false;
                  }
              } else {
                  if (node.num_opts) {
                      dialogo.waiting_for_choice = true;
                  } else {
                      if (dialogo.current_node + 1 < dialogo.num_nodes)
                          dialogo.current_node++;
                  }
              }
          } break;

          //! puxar lógica do menu
          case SDLK_UP: if (dialogo.waiting_for_choice) {
              dialogo.selected_option = (dialogo.selected_option - 1 + node.num_opts) % node.num_opts;
          } break;
          case SDLK_DOWN: if (dialogo.waiting_for_choice) {
              dialogo.selected_option = (dialogo.selected_option + 1) % node.num_opts;
          } break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, PRETO);
              renderDialog(ren, font);
              SDL_RenderPresent(ren);
          } break;
      } break;
    }

    return DIALOGO;
}

void dialogo_free() {
    TTF_CloseFont(font);
    TTF_Quit();
}
