#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>


// faz ser possível usar break/continue num bloco/escopo
#define breakable for (bool __run_ = true; __run_; __run_ = false)

// emula defer usando um goto, uma variável e uma seção de retorno
#define return_defer_var_label(var, val, label) do { \
            (var) = (val); goto label; \
        } while(0)
#define return_defer_var(var, val) return_defer_var_label(var, val, defer)
#define return_defer(val) return_defer_var(ret, val)


long read_file_into_buffer(FILE* f, uint8_t buf[], const size_t len) {
    long err, size, nread;

    err = fseek(f, 0, SEEK_END);
    if (err) return 0;

    size = ftell(f);
    if (size <= 0) return 0;
    if ((size_t)size > len) return len - size;

    if (!buf) return 0;

    err = fseek(f, 0, SEEK_SET);
    if (err != 0) return 0;

    nread = fread(buf, 1, size, f);
    if (nread != size) return 0;

    return nread;
}

size_t read_file_size(FILE* f) {
    const long diff = read_file_into_buffer(f, NULL, 0);
    if (!diff) return 0;

    assert(diff < 0);

    const size_t len = -diff;
    return len;
}

char* slurp_str_from_file(FILE* f, size_t len) {
    char* buf = malloc(len+1);
    assert(buf); buf[len] = '\0';

    long res = read_file_into_buffer(f, (uint8_t*)buf, len);
    fclose(f);

    if (res <= 0) {
        free(buf); buf = NULL;
    }
    return buf;
}

struct lexer {
    char* buf;
    size_t len;
    size_t idx;
    char curr;
    char next;

    const char* file_name;
    size_t line;
    size_t col;
};

enum token_kind {
    ABRE_TITULO = '#',
    FECHA_NOME  = ':',
    ABRE_FALA   = '-',
    ABRE_OPCAO  = '>',
    ABRE_COL    = '[',
    FECHA_COL   = ']',
    ABRE_PAR    = '(',
    FECHA_PAR   = ')',
    FECHA_LINHA = '\n',

    TEXTO = 256,
    INDENTACAO,
    TOK_EOF,
};

char* kind_str(enum token_kind kind) {
    switch (kind) {
      case ABRE_TITULO : return "#";
      case FECHA_NOME  : return ":";
      case ABRE_FALA   : return "-";
      case ABRE_OPCAO  : return ">";
      case ABRE_COL    : return "[";
      case FECHA_COL   : return "]";
      case ABRE_PAR    : return "(";
      case FECHA_PAR   : return ")";
      case FECHA_LINHA : return "\\n";

      case TEXTO       : return "TEXTO";
      case INDENTACAO  : return "INDENTAÇÃO";
      case TOK_EOF     : return "EOF";
    }
    return "TOKEN_INVÁLIDO";
}

struct token_t {
    enum token_kind kind;
    size_t idx, len, line, col;
};

void lexer_reset(struct lexer* l) {
    l->idx = 0;
    l->curr = l->buf[l->idx];
    l->next = l->buf[l->idx+1];
    l->line = l->col = 0;
}

bool lexer_init(struct lexer* l, const char* const file) {
    FILE *f = fopen(file, "rt");
    if (!f) return false;

    size_t len = read_file_size(f);
    if (!len) return false;

    char* buf = slurp_str_from_file(f, len);
    if (!buf) return false;

    l->file_name = file;
    l->buf = buf; l->len = len;
    lexer_reset(l);

    return true;
}

void lexer__advance(struct lexer* l) { //! -> bool?
    switch (l->curr) {
      case '\n': {
          l->line++;
          l->col = 0;
      } break;

      default: l->col++;
    }

    l->idx++;
    l->curr = l->next;
    l->next = l->buf[l->idx+1];
}

bool lexer__is_whitespace(char c) {
    switch (c) {
        default: return false;

        case ' ':
        case '\t':
            return true;
    }
}

bool lexer__is_simple_token(char c) {
    switch (c) {
        default: return false;

        case '#':
        case ':':
        case '-':
        case '>':
        case '[':
        case ']':
        case '(':
        case ')':
        case '\n':
            return true;
    };
}

struct token_t lexer__make_token(struct lexer* start, struct lexer* end,
                                 const enum token_kind kind) {
    //! aqui o fim é no índice do último caracter dentro
    return (struct token_t) {
        .kind = kind,
        .idx = start->idx, .len = end->idx - start->idx + 1,
        .line = start->line, .col = start->col,
    };
}

struct token_t lexer_next(struct lexer* l) {
    struct lexer start = *l;
    char c = l->curr;

    //! decidir se o fim é no próximo caractere ou no atual
    switch (c) {
        case '\0': return lexer__make_token(&start, &start, TOK_EOF);

        case '#':
        case ':':
        case '-':
        case '>':
        case '[':
        case ']':
        case '(':
        case ')':
        case '\n': {
            lexer__advance(l);
            return lexer__make_token(&start, &start, (enum token_kind)c);
        } break;

        case '\v':
        case '\r': { //! lidar melhor
            lexer__advance(l);
            return lexer_next(l);
        } break;

        case ' ':
        case '\t': {
            while (lexer__is_whitespace(l->next)) lexer__advance(l);
            struct lexer end = *l; lexer__advance(l);
            if (start.col != 0) return lexer_next(l);

            return lexer__make_token(&start, &end, INDENTACAO);
        } break;

        default: {
            while (!lexer__is_simple_token(l->next)) lexer__advance(l);
            struct lexer end = *l; lexer__advance(l);
            return lexer__make_token(&start, &end, TEXTO);
        } break;
    }
}

int lexer_fmt_token(struct lexer* lexer, char* buf, struct token_t tok) {
    int len = tok.len, count = 0;
    if (lexer__is_simple_token(tok.kind) || tok.kind == TOK_EOF) len = 0;

    count += sprintf(buf + count, "%s", kind_str(tok.kind));
    if (len)
        count += sprintf(buf + count, "(%.*s)", len, &lexer->buf[tok.idx]);
    return count;
}

void lexer_trace(struct lexer* lexer, struct token_t tok) {
    static char buf[300]; lexer_fmt_token(lexer, buf, tok); //! buffer de tamanho fixo
    printf("%s:%lu:%lu: %s\n", lexer->file_name, tok.line+1, tok.col+1, buf);
}

void lexer_error(struct lexer* lexer, struct token_t* tok, const char* err) {
    size_t line = tok? tok->line: lexer->line,
           col  = tok? tok->col:  lexer->col;
    fprintf(stderr, "%s:%lu:%lu: error: %s\n", lexer->file_name, line+1, col+1, err);
    abort();
}

void lexer_unexpected(struct lexer* lexer, struct token_t tok, enum token_kind kind) {
    const char* fmt = "expected %s, found %s";

    static char buf[300]; //! buffer de tamanho fixo
    lexer_fmt_token(lexer, buf, tok);

    static char err[sizeof(buf) + sizeof(fmt) + sizeof("INDENTAÇÂO")];
    sprintf(err, fmt, kind_str(kind), buf);

    lexer_error(lexer, &tok, err);
}

struct token_t lexer_expect(struct lexer* lexer, enum token_kind kind) {
    struct token_t tok = lexer_next(lexer);
    if (tok.kind != kind) lexer_unexpected(lexer, tok, kind);
    return tok;
}
