#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lex.h"

#define TAM_FONTE 20

#define FUNDO_NOME  CINZA_MEDIO
#define FUNDO_TEXTO CINZA_ESCURO


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
    bool init;
    struct lexer lexer;

    size_t count;
    DialogueNode arvore[300];

    bool waiting;
    size_t opt_idx;
    DialogueNode* current;

    TTF_Font* font;
} dialogo;

void* node_fill(DialogueNode* n, const char* speaker, const char* text) {
    memset(n, 0, sizeof(*n));
    n->speaker = speaker;
    n->text    = text;

    return n;
}

DialogueNode* node_last() {
    return &dialogo.arvore[dialogo.count-1];
}

DialogueNode* node_alloc() {
    assert(dialogo.count < LEN(dialogo.arvore));
    return &dialogo.arvore[dialogo.count++];
}

DialogueNode* node_create(const char* speaker, const char* text) {
    return node_fill(node_alloc(), speaker, text);
}

void node_add_option(DialogueNode* n, const char* text, DialogueNode* next) {
    assert(n->num_opts < LEN(n->options));

    size_t idx = n->num_opts++;
    n->options[idx].text = text;
    n->options[idx].next = next;
}

typedef struct string_view {
    const char* buf;
    size_t len;
} AUX_StringView;

#define AUX_StringViewEq(a, b) (AUX_StringViewCmp(a, b) == 0)
int AUX_StringViewCmp(AUX_StringView a, AUX_StringView b) {
    return strncmp(a.buf, b.buf, a.len);
}

#define AUX_SV(str) ((AUX_StringView){str, strlen(str)})
#define AUX_SVToCstr(str) (strncpy(malloc(str.len+1), str.buf, str.len))

typedef struct {
    AUX_StringView name;
    DialogueNode* first;
} Section;

static struct {
    Section buf[100]; //! tamanho fixo
    size_t len;
} section_table;

Section* section_alloc() {
    assert(section_table.len < LEN(section_table.buf));
    return &section_table.buf[section_table.len++];
}

Section* section_find(AUX_StringView name) {
    for (size_t i = 0; i < LEN(section_table.buf); i++) {
        if (AUX_StringViewEq(name, section_table.buf[i].name)) //! deve ter algo errado aqui
            return &section_table.buf[i];
    }
    return NULL;
}

AUX_StringView curr_speaker = { .buf = NULL, .len = 0 };

void parse_title(struct lexer* l) {
    struct token_t tok;

    tok = lexer_expect(l, TEXTO);
    Section* sec = section_alloc();
    *sec = (Section) {
        .name = (AUX_StringView){ l->buf + tok.idx, tok.len },
        .first = &dialogo.arvore[dialogo.count],
    };

    while (tok.kind != FECHA_LINHA) {
        tok = lexer_next(l);
        sec->name.len += tok.len;
    };

    printf("Seção: %.*s\n", (int)sec->name.len, sec->name.buf);
}

void skip_title(struct lexer* l) {
    struct token_t tok = {0};

    while (tok.kind != FECHA_LINHA && tok.kind != TOK_EOF) {
        tok = lexer_next(l);
    };
}

void parse_fala(struct lexer* l) {
    struct token_t tok = lexer_next(l);

    AUX_StringView str = {l->buf + tok.idx, tok.len};
    while (tok.kind != TOK_EOF) switch (tok.kind) {
        case ABRE_TITULO:
        case FECHA_NOME:
        case ABRE_PAR:  case FECHA_PAR:
        case ABRE_COL:  case FECHA_COL:
        case ABRE_FALA: case ABRE_OPCAO:
        case TEXTO: {
            tok = lexer_next(l);
            str.len += tok.len;
        } break;

        case FECHA_LINHA: {
            printf("- [%.*s]: %.*s\n",
                   (int)curr_speaker.len, curr_speaker.buf,
                   (int)str.len, str.buf);
            DialogueNode* n = node_create(AUX_SVToCstr(curr_speaker),
                                          AUX_SVToCstr(str));
            n->next = n+1; //! errado em muitos momentos

            //! isso tá cagando memória
            ///! (SVToCstr aloca memória e a gente não tenta desduplicar nem os nomes)
            ///! seria bom poder renderizar o texto usando string_view direto
        } return;

        case INDENTACAO: UNREACHABLE();
        case TOK_EOF: return;
    }
}

void parse_option(struct lexer* l) {
    AUX_StringView prev_speaker = curr_speaker;

    struct lexer prev = *l;
    struct token_t tok = lexer_next(l);

    //! isso provavelmente não deveria ser com o mesmo caractere inicial de opção
    if (tok.kind == TEXTO) {
        *l = prev;
        curr_speaker = AUX_SV("(Narrador)"); {
            parse_fala(l);
        } curr_speaker = prev_speaker;
        return;
    }

    *l = prev;
    tok = lexer_expect(l, ABRE_COL);
    tok = lexer_expect(l, TEXTO);
    AUX_StringView str = {l->buf + tok.idx, tok.len};
    tok = lexer_expect(l, FECHA_COL);

    tok = lexer_expect(l, ABRE_PAR);
    tok = lexer_expect(l, TEXTO);
    AUX_StringView lnk = {l->buf + tok.idx, tok.len};
    tok = lexer_expect(l, FECHA_PAR);

    lexer_expect(l, FECHA_LINHA);
    printf("> [%.*s]: [%.*s](%.*s)\n",
           (int)curr_speaker.len, curr_speaker.buf,
           (int)lnk.len, lnk.buf,
           (int)str.len, str.buf);

    DialogueNode* n = node_last(); //! fazer o resto
}

void parse_skip_or_speaker(struct lexer* l) {
    struct lexer prev = *l;
    struct token_t tok = lexer_next(l);
    AUX_StringView nome = {l->buf + tok.idx, tok.len};
    switch (tok.kind) {
        case TEXTO: {
            tok = lexer_expect(l, FECHA_COL);
        } break;
        case FECHA_COL: {
            nome.len = 0;
        } break;

        default: {
            lexer_trace(l, tok);
            UNREACHABLE();
        } break;
    }

    prev = *l;
    tok = lexer_next(l);
    AUX_StringView lnk;
    switch (tok.kind) {
        case FECHA_LINHA: {
            curr_speaker = nome;
            return;
        } break;
        case TEXTO: {
            *l = prev;
            curr_speaker = nome;
            parse_fala(l);
            return;
        } break;
        case ABRE_PAR: {
            tok = lexer_expect(l, TEXTO);
            lnk = (AUX_StringView){l->buf + tok.idx, tok.len};
            tok = lexer_expect(l, FECHA_PAR);
        } break;

        default: {
            lexer_trace(l, tok);
            UNREACHABLE();
        } break;
    }
    printf("[](%.*s)\n", (int)lnk.len, lnk.buf);
}

void skip_comment(struct lexer* l) {
    struct token_t tok = lexer_expect(l, ABRE_TITULO);
    while (tok.kind != TOK_EOF) switch (tok.kind) {
        case ABRE_TITULO:
        case FECHA_NOME:
        case ABRE_PAR:  case FECHA_PAR:
        case ABRE_COL:  case FECHA_COL:
        case ABRE_FALA: case ABRE_OPCAO:
        case TEXTO: {
            tok = lexer_next(l);
        } break;

        case FECHA_LINHA: return;

        case TOK_EOF:
        case INDENTACAO: UNREACHABLE();
    }
}

void parse_dialogue(struct lexer* l) {
    struct token_t tok = {0};
    while (tok.kind != TOK_EOF) {
        tok = lexer_next(l);
        switch (tok.kind) {
          case ABRE_TITULO: parse_title(l);           break;
          case ABRE_FALA:   parse_fala(l);            break;
          case ABRE_OPCAO:  parse_option(l);          break;
          case ABRE_COL:    parse_skip_or_speaker(l); break;

          case INDENTACAO: {
              struct lexer prev = *l;
              tok = lexer_next(l);
              if (tok.kind == ABRE_TITULO) {
                  skip_comment(l);
              } else *l = prev;
          } break;

          case FECHA_LINHA: break;
          case TOK_EOF: return;

          default: {
              lexer_trace(l, tok);
              UNREACHABLE();
          } break;
        }
    }
}


void dialogo_setup(SDL_Renderer* ren) {
    UNUSED(ren);

    TTF_Init();

    dialogo.font = TTF_OpenFont(ASSETS"tiny.ttf", TAM_FONTE);
    dialogo.current = &dialogo.arvore[0];
    dialogo.waiting = false;
    dialogo.opt_idx = 0;
    dialogo.count = 0;

    if (!dialogo.init) {
        lexer_init(&dialogo.lexer, ASSETS"diálogo.md");
        parse_dialogue(&dialogo.lexer);
    }

    dialogo.init = true;

    //! isso é um placeholder
    //DialogueNode *n1, *n2, *n3;
    //n1 = node_create("Alice", "Oi! Voce quer ir ao parque hoje?"); //! acentuação
    //n2 = node_create("Alice", "Otimo! O dia esta lindo la fora."); //! acentuação
    //n3 = node_create("Alice", "Tudo bem, talvez outro dia."); //! acentuação

    //node_add_option(n1, "Sim, vamos!", n2);
    //node_add_option(n1, "Nao, estou cansado.", n3); //! acentuação
}

void dialogo_render(SDL_Renderer* ren, TTF_Font* font) {
    DialogueNode* node = dialogo.current;

    SDL_Rect name = {TAM_FONTE*2, W_WIDTH/2, 10*TAM_FONTE, TAM_FONTE*2 + TAM_FONTE/2};
    SDL_Rect text = {TAM_FONTE*2, name.y + name.h, W_WIDTH - (TAM_FONTE*4), TAM_FONTE*6};
    AUX_SetRenderDrawColor(ren, FUNDO_TEXTO); SDL_RenderFillRect(ren, &text);
    AUX_SetRenderDrawColor(ren, FUNDO_NOME);  SDL_RenderFillRect(ren, &name);

    AUX_DrawTextTTF(ren, font, node->speaker, name.x + 10, name.y + 10);
    AUX_DrawTextTTF(ren, font, node->text,    text.x + 10, text.y + 10);

    if (dialogo.waiting) {
        for (size_t i = 0; i < node->num_opts; ++i) {
            const char* prefix = (i == dialogo.opt_idx ? "> " : "  ");
            AUX_DrawTextTTF(ren, font, prefix,
                            text.x + TAM_FONTE,
                            text.y + TAM_FONTE*3 + i*(TAM_FONTE+5));
            AUX_DrawTextTTF(ren, font, node->options[i].text,
                            text.x + TAM_FONTE*2,
                            text.y + TAM_FONTE*3 + i*(TAM_FONTE+5));
        }
    }
}

enum tela dialogo_loop(SDL_Renderer* ren, SDL_Event evt) {
    const DialogueNode* node = dialogo.current;

    switch (evt.type) {
      case SDL_KEYDOWN: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: return MENU;

          case SDLK_SPACE: case SDLK_RETURN: {
              assert(node->num_opts == 0 || dialogo.opt_idx < node->num_opts);

              if (node->num_opts > 0) {
                  if (!dialogo.waiting) dialogo.waiting = true;
                  else dialogo.current = node->options[dialogo.opt_idx].next;
              } else if (node->next) {
                  dialogo.current = node->next;
                  dialogo.waiting = false;
              }

              dialogo.opt_idx = 0;
          } break;

          //! puxar lógica do menu
          case SDLK_UP: if (node->num_opts > 0) {
              dialogo.opt_idx = (dialogo.opt_idx - 1 + node->num_opts) % node->num_opts;
          } break;
          case SDLK_DOWN: if (node->num_opts > 0) {
              dialogo.opt_idx = (dialogo.opt_idx + 1) % node->num_opts;
          } break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, PRETO); //! colocar fundo
              dialogo_render(ren, dialogo.font);
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
