#include "lexer.h"
#include <ctype.h>
#include <string.h>

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source  = source;
    lexer->current = source;
    lexer->line    = 1;
}

static void skip_whitespace_and_comments(Lexer *lexer) {
    for (;;) {
        while (*lexer->current == ' '  || *lexer->current == '\t' ||
               *lexer->current == '\r' || *lexer->current == '\n') {
            if (*lexer->current == '\n') lexer->line++;
            lexer->current++;
        }
        if (lexer->current[0] == '/' && lexer->current[1] == '/') {
            while (*lexer->current && *lexer->current != '\n') lexer->current++;
            continue;
        }
        if (lexer->current[0] == '/' && lexer->current[1] == '*') {
            lexer->current += 2;
            while (*lexer->current) {
                if (*lexer->current == '\n') lexer->line++;
                if (lexer->current[0] == '*' && lexer->current[1] == '/') {
                    lexer->current += 2;
                    break;
                }
                lexer->current++;
            }
            continue;
        }
        break;
    }
}

static int is_alpha(char c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_alnum(char c) { return isalnum((unsigned char)c) || c == '_'; }
static int is_digit(char c) { return isdigit((unsigned char)c); }

static Token make_token(Lexer *lexer, TokenType type, const char *start) {
    Token t;
    t.type   = type;
    t.start  = start;
    t.length = (int)(lexer->current - start);
    t.line   = lexer->line;
    return t;
}

static TokenType keyword_type(const char *start, int len) {
    #define KW(str, tok) if (len == (int)strlen(str) && strncmp(start, str, len) == 0) return tok
    KW("component", TOKEN_COMPONENT);
    KW("state",     TOKEN_STATE);
    KW("fn",        TOKEN_FN);
    KW("render",    TOKEN_RENDER);
    KW("mount",     TOKEN_MOUNT);
    KW("if",        TOKEN_IF);
    KW("else",      TOKEN_ELSE);
    KW("return",    TOKEN_RETURN);
    KW("let",       TOKEN_LET);
    KW("const",     TOKEN_CONST);
    KW("var",       TOKEN_VAR);
    KW("true",      TOKEN_TRUE);
    KW("false",     TOKEN_FALSE);
    KW("null",      TOKEN_NULL_KW);
    KW("import",    TOKEN_IMPORT);
    KW("from",      TOKEN_FROM);
    KW("export",    TOKEN_EXPORT);
    KW("default",   TOKEN_DEFAULT);
    KW("new",       TOKEN_NEW);
    #undef KW
    return TOKEN_IDENTIFIER;
}

Token lexer_next_token(Lexer *lexer) {
    skip_whitespace_and_comments(lexer);

    if (*lexer->current == '\0') {
        Token t = { TOKEN_EOF, lexer->current, 0, lexer->line };
        return t;
    }

    const char *start = lexer->current;
    char c = *lexer->current++;

    /* identifiers / keywords */
    if (is_alpha(c)) {
        while (is_alnum(*lexer->current)) lexer->current++;
        int len = (int)(lexer->current - start);
        TokenType type = keyword_type(start, len);
        return make_token(lexer, type, start);
    }

    /* numbers */
    if (is_digit(c)) {
        while (is_digit(*lexer->current)) lexer->current++;
        if (*lexer->current == '.' && is_digit(lexer->current[1])) {
            lexer->current++;
            while (is_digit(*lexer->current)) lexer->current++;
        }
        return make_token(lexer, TOKEN_NUMBER, start);
    }

    /* strings */
    if (c == '"' || c == '\'') {
        char quote = c;
        while (*lexer->current && *lexer->current != quote) {
            if (*lexer->current == '\\') lexer->current++;
            if (*lexer->current == '\n') lexer->line++;
            lexer->current++;
        }
        if (*lexer->current == quote) lexer->current++;
        return make_token(lexer, TOKEN_STRING, start);
    }

    /* two-character operators */
    char n = *lexer->current;
    if (c == '<' && n == '/') { lexer->current++; return make_token(lexer, TOKEN_LT_SLASH, start); }
    if (c == '/' && n == '>') { lexer->current++; return make_token(lexer, TOKEN_SLASH_GT, start); }
    if (c == '=' && n == '=') { lexer->current++; return make_token(lexer, TOKEN_EQ,           start); }
    if (c == '!' && n == '=') { lexer->current++; return make_token(lexer, TOKEN_NEQ,          start); }
    if (c == '<' && n == '=') { lexer->current++; return make_token(lexer, TOKEN_LTE,          start); }
    if (c == '>' && n == '=') { lexer->current++; return make_token(lexer, TOKEN_GTE,          start); }
    if (c == '&' && n == '&') { lexer->current++; return make_token(lexer, TOKEN_AND,          start); }
    if (c == '|' && n == '|') { lexer->current++; return make_token(lexer, TOKEN_OR,           start); }
    if (c == '+' && n == '+') { lexer->current++; return make_token(lexer, TOKEN_PLUS_PLUS,    start); }
    if (c == '-' && n == '-') { lexer->current++; return make_token(lexer, TOKEN_MINUS_MINUS,  start); }
    if (c == '+' && n == '=') { lexer->current++; return make_token(lexer, TOKEN_PLUS_ASSIGN,  start); }
    if (c == '-' && n == '=') { lexer->current++; return make_token(lexer, TOKEN_MINUS_ASSIGN, start); }
    if (c == '=' && n == '>') { lexer->current++; return make_token(lexer, TOKEN_ARROW,        start); }

    /* single-character operators */
    switch (c) {
        case '=': return make_token(lexer, TOKEN_ASSIGN,    start);
        case '+': return make_token(lexer, TOKEN_PLUS,      start);
        case '-': return make_token(lexer, TOKEN_MINUS,     start);
        case '*': return make_token(lexer, TOKEN_STAR,      start);
        case '/': return make_token(lexer, TOKEN_SLASH,     start);
        case '%': return make_token(lexer, TOKEN_PERCENT,   start);
        case '<': return make_token(lexer, TOKEN_LT,        start);
        case '>': return make_token(lexer, TOKEN_GT,        start);
        case '!': return make_token(lexer, TOKEN_NOT,       start);
        case '@': return make_token(lexer, TOKEN_AT,        start);
        case '{': return make_token(lexer, TOKEN_LBRACE,    start);
        case '}': return make_token(lexer, TOKEN_RBRACE,    start);
        case '(': return make_token(lexer, TOKEN_LPAREN,    start);
        case ')': return make_token(lexer, TOKEN_RPAREN,    start);
        case '[': return make_token(lexer, TOKEN_LBRACKET,  start);
        case ']': return make_token(lexer, TOKEN_RBRACKET,  start);
        case ';': return make_token(lexer, TOKEN_SEMICOLON, start);
        case ',': return make_token(lexer, TOKEN_COMMA,     start);
        case '.': return make_token(lexer, TOKEN_DOT,       start);
        case '?': return make_token(lexer, TOKEN_QUESTION,  start);
        case ':': return make_token(lexer, TOKEN_COLON,     start);
        default:  return make_token(lexer, TOKEN_ERROR,     start);
    }
}

Token lexer_peek_token(Lexer *lexer) {
    Lexer saved = *lexer;
    Token t = lexer_next_token(lexer);
    *lexer = saved;
    return t;
}

const char *token_type_name(TokenType t) {
    switch (t) {
        case TOKEN_COMPONENT:    return "component";
        case TOKEN_STATE:        return "state";
        case TOKEN_FN:           return "fn";
        case TOKEN_RENDER:       return "render";
        case TOKEN_MOUNT:        return "mount";
        case TOKEN_IF:           return "if";
        case TOKEN_ELSE:         return "else";
        case TOKEN_RETURN:       return "return";
        case TOKEN_LET:          return "let";
        case TOKEN_CONST:        return "const";
        case TOKEN_VAR:          return "var";
        case TOKEN_TRUE:         return "true";
        case TOKEN_FALSE:        return "false";
        case TOKEN_NULL_KW:      return "null";
        case TOKEN_IDENTIFIER:   return "identifier";
        case TOKEN_NUMBER:       return "number";
        case TOKEN_STRING:       return "string";
        case TOKEN_ASSIGN:       return "=";
        case TOKEN_EQ:           return "==";
        case TOKEN_NEQ:          return "!=";
        case TOKEN_LT:           return "<";
        case TOKEN_GT:           return ">";
        case TOKEN_LTE:          return "<=";
        case TOKEN_GTE:          return ">=";
        case TOKEN_AND:          return "&&";
        case TOKEN_OR:           return "||";
        case TOKEN_NOT:          return "!";
        case TOKEN_AT:           return "@";
        case TOKEN_PLUS:         return "+";
        case TOKEN_MINUS:        return "-";
        case TOKEN_STAR:         return "*";
        case TOKEN_SLASH:        return "/";
        case TOKEN_LBRACE:       return "{";
        case TOKEN_RBRACE:       return "}";
        case TOKEN_LPAREN:       return "(";
        case TOKEN_RPAREN:       return ")";
        case TOKEN_SEMICOLON:    return ";";
        case TOKEN_COMMA:        return ",";
        case TOKEN_EOF:          return "EOF";
        default:                 return "?";
    }
}
