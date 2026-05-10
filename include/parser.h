#ifndef SUN_PARSER_H
#define SUN_PARSER_H

#include "lexer.h"

typedef enum {
    AST_VAR_DECL,
    AST_FUNC_DECL,
    AST_SPEC_DECL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *name;
    struct ASTNode *body; // Simplified for now
    struct ASTNode *next;
} ASTNode;

typedef struct {
    Lexer *lexer;
    Token current;
} Parser;

void parser_init(Parser *parser, Lexer *lexer);
ASTNode* parser_parse(Parser *parser);
void parser_free(ASTNode *node);

void generate_c(ASTNode *node);

#endif // SUN_PARSER_H
