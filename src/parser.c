#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->current = lexer_next_token(lexer);
}

static void advance(Parser *parser) {
    parser->current = lexer_next_token(parser->lexer);
}

static ASTNode* create_node(ASTNodeType type, const char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->name = strdup(name);
    node->body = NULL;
    node->next = NULL;
    return node;
}

static ASTNode* parse_declaration(Parser *parser) {
    if (parser->current.type == TOKEN_FUNC) {
        advance(parser);
        if (parser->current.type == TOKEN_IDENTIFIER) {
            ASTNode *node = create_node(AST_FUNC_DECL, "func_name");
            advance(parser);
            return node;
        }
    } else if (parser->current.type == TOKEN_SPEC) {
        advance(parser);
        if (parser->current.type == TOKEN_IDENTIFIER) {
            ASTNode *node = create_node(AST_SPEC_DECL, "spec_name");
            advance(parser);
            return node;
        }
    } else if (parser->current.type == TOKEN_LET || 
               parser->current.type == TOKEN_VAR || 
               parser->current.type == TOKEN_CONST) {
        advance(parser);
        if (parser->current.type == TOKEN_IDENTIFIER) {
            ASTNode *node = create_node(AST_VAR_DECL, "var_name");
            advance(parser);
            return node;
        }
    }
    return NULL;
}

ASTNode* parser_parse(Parser *parser) {
    ASTNode *head = NULL;
    ASTNode *current = NULL;

    while (parser->current.type != TOKEN_EOF) {
        ASTNode *node = parse_declaration(parser);
        if (node) {
            if (!head) head = node;
            else current->next = node;
            current = node;
        } else {
            advance(parser); // Skip unknown
        }
    }
    return head;
}

void parser_free(ASTNode *node) {
    while (node) {
        ASTNode *next = node->next;
        free(node->name);
        free(node);
        node = next;
    }
}
