#include "lexer.h"
#include <ctype.h>
#include <string.h>

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->current = source;
}

static int is_alpha(char c) {
    return isalpha(c) || c == '_';
}

static int is_digit(char c) {
    return isdigit(c);
}

Token lexer_next_token(Lexer *lexer) {
    while (*lexer->current == ' ' || *lexer->current == '\t' || *lexer->current == '\n' || *lexer->current == '\r') {
        lexer->current++;
    }

    if (*lexer->current == '\0') return (Token){TOKEN_EOF, lexer->current, 0};

    const char *start = lexer->current;

    if (is_alpha(*lexer->current)) {
        while (is_alpha(*lexer->current) || is_digit(*lexer->current)) lexer->current++;
        int len = lexer->current - start;
        if (strncmp(start, "let", len) == 0 && len == 3) return (Token){TOKEN_LET, start, len};
        if (strncmp(start, "var", len) == 0 && len == 3) return (Token){TOKEN_VAR, start, len};
        if (strncmp(start, "const", len) == 0 && len == 5) return (Token){TOKEN_CONST, start, len};
        if (strncmp(start, "func", len) == 0 && len == 4) return (Token){TOKEN_FUNC, start, len};
        if (strncmp(start, "spec", len) == 0 && len == 4) return (Token){TOKEN_SPEC, start, len};
        return (Token){TOKEN_IDENTIFIER, start, len};
    }

    if (is_digit(*lexer->current)) {
        while (is_digit(*lexer->current)) lexer->current++;
        return (Token){TOKEN_NUMBER, start, lexer->current - start};
    }

    if (*lexer->current == '=') {
        lexer->current++;
        return (Token){TOKEN_ASSIGN, start, 1};
    }

    return (Token){TOKEN_ERROR, start, 1};
}
