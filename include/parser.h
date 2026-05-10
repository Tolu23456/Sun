#ifndef SUN_PARSER_H
#define SUN_PARSER_H

#include "lexer.h"
#include "sun.h"

typedef enum {
    AST_PROGRAM,
    AST_COMPONENT_DECL,
    AST_STATE_DECL,
    AST_FN_DECL,
    AST_FN_PARAM,
    AST_RENDER_BLOCK,
    AST_MOUNT_CALL,
    AST_IMPORT_DECL,

    /* statements */
    AST_BLOCK,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_RETURN_STMT,
    AST_EXPR_STMT,
    AST_VAR_DECL_STMT,

    /* expressions */
    AST_ASSIGN,
    AST_BINARY,
    AST_UNARY,
    AST_CALL,
    AST_MEMBER,
    AST_TERNARY,
    AST_IDENT,
    AST_NUMBER,
    AST_STRING,
    AST_BOOL,
    AST_NULL_LIT,

    /* template */
    AST_ELEMENT,
    AST_TEXT,
    AST_INTERPOLATION,
    AST_ATTR,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType  type;

    /* value storage */
    char        *str_val;   /* name, string literal, operator */
    double       num_val;
    int          bool_val;

    /* general-purpose children */
    struct ASTNode *left;      /* binary/unary operand, assign lhs, condition */
    struct ASTNode *right;     /* binary rhs, assign rhs */
    struct ASTNode *body;      /* block / then-branch */
    struct ASTNode *else_br;   /* else branch */
    struct ASTNode *args;      /* call args linked via ->next */
    struct ASTNode *params;    /* fn params linked via ->next */
    struct ASTNode *attrs;     /* element attrs linked via ->next */
    struct ASTNode *children;  /* element children linked via ->next */
    struct ASTNode *members;   /* component members linked via ->next */
    struct ASTNode *next;      /* sibling in any linked list */

    int            line;
} ASTNode;

typedef struct {
    Lexer      *lexer;
    Token       current;
    Token       peek;
    SunErrors  *errors;
    int         had_error;
} Parser;

void     parser_init(Parser *parser, Lexer *lexer, SunErrors *errors);
ASTNode *parser_parse(Parser *parser);
void     ast_free(ASTNode *node);
void     ast_print(ASTNode *node, int depth);

#endif
