#ifndef SUN_LEXER_H
#define SUN_LEXER_H

typedef enum {
    /* keywords */
    TOKEN_COMPONENT,
    TOKEN_PAGE,
    TOKEN_STATE,
    TOKEN_FN,
    TOKEN_RENDER,
    TOKEN_STYLE,
    TOKEN_MOUNT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_VAR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL_KW,
    TOKEN_IMPORT,
    TOKEN_FROM,
    TOKEN_EXPORT,
    TOKEN_DEFAULT,
    TOKEN_NEW,

    /* literals */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

    /* operators */
    TOKEN_ASSIGN,        /* =  */
    TOKEN_PLUS,          /* +  */
    TOKEN_MINUS,         /* -  */
    TOKEN_STAR,          /* *  */
    TOKEN_SLASH,         /* /  */
    TOKEN_PERCENT,       /* %  */
    TOKEN_EQ,            /* == */
    TOKEN_NEQ,           /* != */
    TOKEN_LT,            /* <  */
    TOKEN_GT,            /* >  */
    TOKEN_LTE,           /* <= */
    TOKEN_GTE,           /* >= */
    TOKEN_AND,           /* && */
    TOKEN_OR,            /* || */
    TOKEN_NOT,           /* !  */
    TOKEN_PLUS_PLUS,     /* ++ */
    TOKEN_MINUS_MINUS,   /* -- */
    TOKEN_PLUS_ASSIGN,   /* += */
    TOKEN_MINUS_ASSIGN,  /* -= */
    TOKEN_ARROW,         /* => */
    TOKEN_QUESTION,      /* ?  */
    TOKEN_COLON,         /* :  */

    /* punctuation */
    TOKEN_AT,            /* @  */
    TOKEN_LBRACE,        /* {  */
    TOKEN_RBRACE,        /* }  */
    TOKEN_LPAREN,        /* (  */
    TOKEN_RPAREN,        /* )  */
    TOKEN_LBRACKET,      /* [  */
    TOKEN_RBRACKET,      /* ]  */
    TOKEN_SEMICOLON,     /* ;  */
    TOKEN_COMMA,         /* ,  */
    TOKEN_DOT,           /* .  */

    /* template-specific */
    TOKEN_LT_SLASH,      /* </ */
    TOKEN_SLASH_GT,      /* /> */

    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType   type;
    const char *start;
    int         length;
    int         line;
} Token;

typedef struct {
    const char *source;
    const char *current;
    int         line;
} Lexer;

void  lexer_init(Lexer *lexer, const char *source);
Token lexer_next_token(Lexer *lexer);
Token lexer_peek_token(Lexer *lexer);

const char *token_type_name(TokenType t);

#endif
