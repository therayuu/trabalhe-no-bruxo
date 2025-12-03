#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>

#define TAM_FONTE 20


typedef struct dialog_option DialogueOption;
typedef struct dialog_node   DialogueNode;

struct dialog_option {
    const char* text;
    DialogueNode* next;
};

struct dialog_node {
    const char* speaker;
    const char* text;

    size_t num_opts;
    union {
        DialogueNode* next;
        DialogueOption options[5]; //! tamanho hardcoded
    };
};

static struct estado_dialogo {
    size_t count;
    DialogueNode arvore[300];

    size_t selected_option;
    DialogueNode* current;
    bool waiting_for_choice; //! esse campo não deveria existir

    TTF_Font* font;
} dialogo;


void* fillNode(DialogueNode* n, const char* speaker, const char* text) {
    memset(n, 0, sizeof(*n));
    n->speaker = speaker;
    n->text    = text;

    return n;
}

DialogueNode* newNode() {
    assert(dialogo.count < LEN(dialogo.arvore));
    return &dialogo.arvore[dialogo.count++];
}

DialogueNode* createNode(const char* speaker, const char* text) {
    return fillNode(newNode(), speaker, text);
}

void addOption(DialogueNode* n, const char* text, DialogueNode* next) {
    assert(n->num_opts < LEN(n->options));
    size_t idx = n->num_opts++;
    n->options[idx].text = text;
    n->options[idx].next = next;
}

void loadDialogue() {
    DialogueNode *n1, *n2, *n3;
    n1 = createNode("Alice", "Oi! Voce quer ir ao parque hoje?"); //! acentuação
    n2 = createNode("Alice", "Otimo! O dia esta lindo la fora."); //! acentuação
    n3 = createNode("Alice", "Tudo bem, talvez outro dia."); //! acentuação

    addOption(n1, "Sim, vamos!", n2);
    addOption(n1, "Nao, estou cansado.", n3); //! acentuação
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
    DialogueNode* node = dialogo.current;

    SDL_Rect nameBox = {TAM_FONTE*2, W_WIDTH/2, 10*TAM_FONTE, TAM_FONTE*2 + TAM_FONTE/2};
    SDL_Rect textBox = {TAM_FONTE*2, nameBox.y + nameBox.h, W_WIDTH - (TAM_FONTE*4), TAM_FONTE*6};

    SDL_SetRenderDrawColor(ren, 50, 50, 50, 255); SDL_RenderFillRect(ren, &textBox);
    SDL_SetRenderDrawColor(ren, 80, 80, 80, 255); SDL_RenderFillRect(ren, &nameBox);

    renderText(ren, font, node->speaker, nameBox.x + 10, nameBox.y + 10);
    renderText(ren, font, node->text,    textBox.x + 10, textBox.y + 10);

    if (dialogo.waiting_for_choice) {
        for (size_t i = 0; i < node->num_opts; ++i) {
            const char* prefix = (i == dialogo.selected_option ? "> " : "  ");
            renderText(ren, font, prefix,
                       textBox.x + TAM_FONTE,
                       textBox.y + TAM_FONTE*3 + i*(TAM_FONTE+5));
            renderText(ren, font, node->options[i].text,
                       textBox.x + TAM_FONTE*2,
                       textBox.y + TAM_FONTE*3 + i*(TAM_FONTE+5));
        }
    }
}


void dialogo_setup(SDL_Renderer* ren) {
    UNUSED(ren);

    TTF_Init();

    dialogo.font = TTF_OpenFont(ASSETS"tiny.ttf", TAM_FONTE);
    dialogo.current = &dialogo.arvore[0];
    dialogo.waiting_for_choice = false;
    dialogo.selected_option = 0;
    dialogo.count = 0;

    loadDialogue();
}

enum tela dialogo_loop(SDL_Renderer* ren, SDL_Event evt) {
    const DialogueNode* node = dialogo.current;

    switch (evt.type) {
      case SDL_KEYDOWN: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: return MENU;

          case SDLK_SPACE: case SDLK_RETURN: {
              if (dialogo.waiting_for_choice) {
                  if (dialogo.selected_option < node->num_opts) {
                      dialogo.current = node->options[dialogo.selected_option].next;
                      dialogo.waiting_for_choice = false;
                  }
              } else {
                  if (node->num_opts > 0) {
                      dialogo.waiting_for_choice = true;
                  } else if (node->next) {
                      dialogo.current = node->next;
                  }
              }
          } break;

          //! puxar lógica do menu
          case SDLK_UP: if (dialogo.waiting_for_choice) {
              dialogo.selected_option = (dialogo.selected_option - 1 + node->num_opts) % node->num_opts;
          } break;

          //! puxar lógica do menu
          case SDLK_DOWN: if (dialogo.waiting_for_choice) {
              dialogo.selected_option = (dialogo.selected_option + 1) % node->num_opts;
          } break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, PRETO);
              renderDialog(ren, dialogo.font);
              SDL_RenderPresent(ren);
          } break;
      } break;
    }

    return DIALOGO;
}

void dialogo_free() {
    dialogo.count = 0;

    TTF_CloseFont(dialogo.font);
    TTF_Quit();
}
