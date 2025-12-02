#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>

#define TAM_FONTE 20

typedef struct dialog_node DialogueNode;

typedef struct dialog_option {
    char* text;
    DialogueNode* nextNode;   //ponteiro para o próximo dialogo
} DialogueOption;

struct dialog_node {
    char* speaker;
    char* text;
    size_t num_opts;
    DialogueOption options[5];  //! tamanho hardcoded  
    DialogueNode* next;
   
};

TTF_Font* font;

static struct estado_dialogo {
    DialogueNode* head;        // início da lista
    DialogueNode* current;     // nó atual
    bool waiting_for_choice;   //! esse campo não deveria existir
    size_t selected_option;
} dialogo;



DialogueNode* createNode(const char* speaker, const char* text)
{
    DialogueNode* n = malloc(sizeof(DialogueNode));
    n->speaker = (char*)speaker;
    n->text = (char*)text;
    n->num_opts = 0;
    n->next = NULL;
    return n;
}



void loadDialogue()
  // Exemplo simples de estrutura de diálogo
{
    // Criar nós
    DialogueNode* n1 = createNode(
        "Alice",
        "Oi! Voce "/*"ê"*/"quer ir ao parque hoje?"
    );

    DialogueNode* n2 = createNode(
        "Alice",
        /*"Ó"*/"Otimo! O dia esta " /*"á"*/ "lindo la " /*"á"*/ "fora."
    );

    DialogueNode* n3 = createNode(
        "Alice",
        "Tudo bem, talvez outro dia."
    );

    // Construir lista encadeada linear
    n1->next = n2;   // caso escolha 0

    // Adicionar opções
    n1->options[n1->num_opts++] = (DialogueOption){ "Sim, vamos!", n2 };
    n1->options[n1->num_opts++] = (DialogueOption){ "Nao, estou cansado.", n3 };

    // HEAD da lista
    dialogo.head = n1;
    dialogo.current = n1;
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

    SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
    SDL_RenderFillRect(ren, &textBox);

    SDL_SetRenderDrawColor(ren, 80, 80, 80, 255);
    SDL_RenderFillRect(ren, &nameBox);

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
    (void)ren;

    dialogo.waiting_for_choice = false;
    dialogo.selected_option = 0;

    TTF_Init();
    font = TTF_OpenFont(ASSETS"tiny.ttf", TAM_FONTE);

    loadDialogue();
}



enum tela dialogo_loop(SDL_Renderer* ren, SDL_Event evt) {
    DialogueNode* node = dialogo.current;
    enum tela prox_tela = DIALOGO;
    switch (evt.type) {

      case SDL_KEYDOWN:
          switch (evt.key.keysym.sym) {

              case SDLK_ESCAPE:
                  prox_tela = MENU; break;

              case SDLK_SPACE:
              case SDLK_RETURN:
              {
                  if (dialogo.waiting_for_choice) {
                      if (dialogo.selected_option < node->num_opts) {
                          dialogo.current = node->options[dialogo.selected_option].nextNode;
                          dialogo.waiting_for_choice = false;
                      }
                  } else {
                      if (node->num_opts > 0) {
                          dialogo.waiting_for_choice = true;
                      } else if (node->next) {
                          dialogo.current = node->next;
                      }
                  }
              }
              break;

	       //! puxar lógica do menu
              case SDLK_UP:
                  if (dialogo.waiting_for_choice)
                      dialogo.selected_option =
                          (dialogo.selected_option - 1 + node->num_opts) % node->num_opts;
              break;

              case SDLK_DOWN:
                  if (dialogo.waiting_for_choice)
                      dialogo.selected_option =
                          (dialogo.selected_option + 1) % node->num_opts;
              break;
          }
      break;

      case SDL_USEREVENT:
          if (evt.user.code == AUX_TIMEOUTEVENT) {
              AUX_RenderClearColor(ren, PRETO);
              renderDialog(ren, font);
              SDL_RenderPresent(ren);
          }
      break;
    }

    return prox_tela;
}



void freeDialogueList(DialogueNode* head)
{
    while (head) {
        DialogueNode* next = head->next;
        free(head);
        head = next;
    }
}

void dialogo_free() {
    freeDialogueList(dialogo.head);

    TTF_CloseFont(font);
    TTF_Quit();
}
