#ifndef SUN_LEXER_H
#define SUN_LEXER_H

typedef enum {
    TOKEN_LET,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_FUNC,
    TOKEN_SPEC,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_ASSIGN,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
} Token;

typedef struct {
    const char *source;
    const char *current;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next_token(Lexer *lexer);

#endif // SUN_LEXER_H
