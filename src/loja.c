#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lex.h"

#define TAM_FONTE (W_HEIGHT/22)

#define FUNDO_NOME  CINZA_MEDIO
#define FUNDO_TEXTO CINZA_ESCURO


typedef struct dialog_option DialogueOption;
typedef struct dialog_node   DialogueNode;

struct dialog_option {
    const char* text;
    DialogueNode* next;

    AUX_StringView sect;
};

struct dialog_node {
    const char* speaker;
    const char* text;

    bool action;

    size_t num_opts;
    union {
        DialogueNode* next;
        DialogueOption opts[4]; //! tamanho hardcoded
    };
};

static struct estado_loja {
    bool init;
    struct lexer lexer;

    size_t count;
    DialogueNode arvore[300];

    bool choosing;
    size_t opt_idx;
    DialogueNode* current;

    TTF_Font* font;
} loja;

void* node_fill(DialogueNode* n, const char* speaker, const char* text) {
    memset(n, 0, sizeof(*n));
    n->speaker = speaker;
    n->text    = text;

    return n;
}

DialogueNode* node_last() {
    return &loja.arvore[loja.count-1];
}

DialogueNode* node_alloc() {
    assert(loja.count < LEN(loja.arvore));
    return &loja.arvore[loja.count++];
}

DialogueNode* node_create(const char* speaker, const char* text) {
    return node_fill(node_alloc(), speaker, text);
}

void node_add_option(DialogueNode* n, const char* text, AUX_StringView sect) {
    assert(n->num_opts < LEN(n->opts));

    size_t idx = n->num_opts++;
    n->opts[idx].text = text;
    n->opts[idx].next = NULL;
    n->opts[idx].sect = sect;
}

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

//! tá claramente fora de lugar
//! devia usar um array local e mandar o ponteiro via evento
void pocoes_possiveis_add(const char* s) {
    size_t len = AUX_NullTerminatedLen(pocoes_possiveis);
    assert(len+1 < LEN(pocoes_possiveis));
    pocoes_possiveis[len] = tipo_carta_from_str(s);
    pocoes_possiveis[len+1] = 0;
}

size_t pocoes_possiveis_find(enum tipo_carta carta) { //! AUX_NullTerminatedIndex
    for (size_t i = 0; pocoes_possiveis[i]; i++) {
        if (pocoes_possiveis[i] == carta) return i;
    }
    return SIZE_MAX;
}


AUX_StringView curr_speaker = {0};

void parse_title(struct lexer* l) {
    struct token_t tok;

    tok = lexer_expect(l, TEXTO);
    Section* sec = section_alloc();
    *sec = (Section) {
        .name = (AUX_StringView){ l->buf + tok.idx, tok.len },
        .first = &loja.arvore[loja.count],
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
    tok = lexer_next(l);
    switch (tok.kind) {
        case FECHA_COL: break;
        case FECHA_NOME: {
            assert(AUX_StringViewEq(str, AUX_SV("MESA")));
            node_last()->action = true;

            tok = lexer_expect(l, TEXTO);
            str = (AUX_StringView){l->buf + tok.idx, tok.len};
            tok = lexer_expect(l, FECHA_COL);
        } break;
        default: {
            lexer_trace(l, tok);
            UNREACHABLE();
        } break;
    }

    tok = lexer_expect(l, ABRE_PAR);
    tok = lexer_expect(l, TEXTO);
    AUX_StringView lnk = {l->buf + tok.idx, tok.len};
    tok = lexer_expect(l, FECHA_PAR);

    lexer_expect(l, FECHA_LINHA);
    printf("> [%.*s]: [%.*s](%.*s)\n",
           (int)curr_speaker.len, curr_speaker.buf,
           (int)lnk.len, lnk.buf,
           (int)str.len, str.buf);

    node_add_option(node_last(), AUX_SVToCstr(str), lnk);
}

void parse_speaker_or_redirect(struct lexer* l) {
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

    AUX_StringView lnk;
    if (nome.len == 0) {
        tok = lexer_expect(l, ABRE_PAR);
        tok = lexer_expect(l, TEXTO);
        lnk = (AUX_StringView){l->buf + tok.idx, tok.len};
        tok = lexer_expect(l, FECHA_PAR);

        assert(lnk.buf && lnk.len < 100);
        printf("[](%.*s)\n", (int)lnk.len, lnk.buf);
        node_add_option(node_create(NULL, NULL), NULL, lnk);

        return;
    } else

    prev = *l;
    tok = lexer_next(l);
    switch (tok.kind) {
        case FECHA_LINHA: {
            assert(nome.len != 0);
            curr_speaker = nome;
        } return;
        default: {
            assert(nome.len != 0);
            curr_speaker = nome;

            *l = prev; parse_fala(l);
        } return;
    }
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
          case ABRE_TITULO: parse_title(l);               break;
          case ABRE_FALA:   parse_fala(l);                break;
          case ABRE_OPCAO:  parse_option(l);              break;
          case ABRE_COL:    parse_speaker_or_redirect(l); break;

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


void loja_setup(SDL_Renderer* ren) {
    UNUSED(ren);

    TTF_Init();

    loja.font = TTF_OpenFont(ASSETS"básica-unicode-regular.ttf", TAM_FONTE);
    if (!loja.init) {
        loja.current = &loja.arvore[0];
        loja.choosing = false;
        loja.opt_idx = 0;
        loja.count = 0;

        lexer_init(&loja.lexer, ASSETS"diálogo.mvp.md");
        parse_dialogue(&loja.lexer);
    }
    loja.init = true;
}

void loja_render(SDL_Renderer* ren, TTF_Font* font) {
    DialogueNode* node = loja.current;

    int font_w, font_h;
    TTF_SizeUTF8(font, "O", &font_w, &font_h);

    const int wpad = font_w, hpad = font_h/2;
    SDL_Rect text = { .x=wpad, .w = W_WIDTH - (wpad*2), .h = (font_h + hpad/2)*5 };
    SDL_Rect name = { .x=wpad, .w = 10*font_w,          .h = (font_h + hpad*2)   };

    text.y = W_HEIGHT - text.h - hpad;
    name.y = text.y - name.h;

    AUX_SetRenderDrawColor(ren, FUNDO_TEXTO); SDL_RenderFillRect(ren, &text);
    AUX_SetRenderDrawColor(ren, FUNDO_NOME);  SDL_RenderFillRect(ren, &name);

    AUX_DrawTextTTFWrap(ren, font, node->speaker, name.x + wpad/2, name.y + hpad, 0);
    AUX_DrawTextTTFWrap(ren, font, node->text,    text.x + wpad/2, text.y + hpad,
                        text.w - wpad);

    if (loja.choosing && !node->action) {
        char buf[300];
        for (size_t i = 0; i < node->num_opts; ++i) {
            const char prefix = i==loja.opt_idx ? '>' : ' ';
            const DialogueOption opt = node->opts[i];

            sprintf(buf, "%c %s", prefix, opt.text);
            AUX_DrawTextTTF(ren, font, buf,
                text.x + wpad/2 + font_w + wpad/2,
                text.y + text.h - (
                    hpad + (node->num_opts-i)*(font_h + hpad/2)
                )
            );
        }
    }
}

enum tela loja_loop(SDL_Renderer* ren, SDL_Event evt) {
    DialogueNode* node = loja.current;
    if (!node->speaker && !node->text) {
        assert(node->num_opts == 1);
        assert(node->opts[0].text == NULL);

        DialogueOption* opt = &node->opts[loja.opt_idx];
        if (!opt->next) opt->next = section_find(opt->sect)->first;
        loja.current = node = opt->next;
    }

    switch (evt.type) {
      case SDL_KEYDOWN: switch (evt.key.keysym.sym) {
          case SDLK_ESCAPE: return MENU;

          case SDLK_SPACE: case SDLK_RETURN: {
              assert(node->num_opts == 0 || loja.opt_idx < node->num_opts);

              if (node->num_opts > 0) {
                  DialogueOption* opt = &node->opts[loja.opt_idx];
                  if (!opt->next) opt->next = section_find(opt->sect)->first;

                  if (!loja.choosing) {
                      loja.choosing = true;
                      if (node->action) {
                          pocoes_possiveis[0] = 0;
                          for (size_t i = 0; i < node->num_opts; i++)
                              pocoes_possiveis_add(node->opts[i].text);
                          return MESA;
                      }
                  } else if (!node->action) loja.current = opt->next;
              } else if (node->next) {
                  loja.current = node->next;
                  loja.choosing = false;
              }

              loja.opt_idx = 0;
          } break;

          //! puxar lógica do menu
          case SDLK_UP: if (node->num_opts > 0 && !node->action) {
              loja.opt_idx = (loja.opt_idx - 1 + node->num_opts) % node->num_opts;
          } break;
          case SDLK_DOWN: if (node->num_opts > 0 && !node->action) {
              loja.opt_idx = (loja.opt_idx + 1) % node->num_opts;
          } break;
      } break;

      case SDL_USEREVENT: switch (evt.user.code) {
          case AUX_MERGEEVENT: {
              assert(node->action);
              loja.opt_idx = pocoes_possiveis_find((enum tipo_carta)evt.user.data1);

              DialogueOption* opt = &node->opts[loja.opt_idx];
              if (!opt->next) opt->next = section_find(opt->sect)->first;
              loja.current = opt->next;
              loja.choosing = false;
          } break;
          case AUX_TIMEOUTEVENT: {
              AUX_RenderClearColor(ren, PRETO); //! colocar fundo
              loja_render(ren, loja.font);
              SDL_RenderPresent(ren);
          } break;
      } break;
    }

    return LOJA;
}

void loja_free() {
    loja.count = 0;

    TTF_CloseFont(loja.font);
    TTF_Quit();
}
