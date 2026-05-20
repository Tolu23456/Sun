#include "parser.h"
#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

void bc_init(BytecodeBuffer *bb) {
    bb->capacity = 1024;
    bb->size = 0;
    bb->code = malloc(bb->capacity);
}

void bc_emit(BytecodeBuffer *bb, uint8_t byte) {
    if (bb->size >= bb->capacity) {
        bb->capacity *= 2;
        bb->code = realloc(bb->code, bb->capacity);
    }
    bb->code[bb->size++] = byte;
}

void bc_generate_expr(BytecodeBuffer *bb, ASTNode *n) {
    if (!n) return;
    switch (n->type) {
        case AST_NUMBER:
            bc_emit(bb, OP_LOAD_CONST);
            /* simplified: push index or small value */
            bc_emit(bb, (uint8_t)n->num_val);
            break;
        case AST_BINARY:
            bc_generate_expr(bb, n->left);
            bc_generate_expr(bb, n->right);
            if (strcmp(n->str_val, "+") == 0) bc_emit(bb, OP_ADD);
            else if (strcmp(n->str_val, "-") == 0) bc_emit(bb, OP_SUB);
            break;
        default: break;
    }
}

static void bc_gen_tmpl(BytecodeBuffer *bb, ASTNode *n) {
    if (!n) return;
    switch (n->type) {
        case AST_ELEMENT:
            if (strcmp(n->str_val, "div") == 0) bc_emit(bb, OP_PRIMITIVE_DIV);
            else if (strcmp(n->str_val, "button") == 0) bc_emit(bb, OP_PRIMITIVE_BUTTON);
            else if (strcmp(n->str_val, "span") == 0) bc_emit(bb, OP_PRIMITIVE_SPAN);
            else if (strcmp(n->str_val, "nav") == 0) bc_emit(bb, OP_PRIMITIVE_NAV);
            else { bc_emit(bb, OP_CALL_COMPONENT); }

            ASTNode *child = n->children;
            while (child) { bc_gen_tmpl(bb, child); child = child->next; }
            break;
        case AST_TEXT: bc_emit(bb, OP_PRIMITIVE_TEXT); break;
        case AST_INTERPOLATION: bc_emit(bb, OP_LOAD_STATE); break;
        default: break;
    }
}

void bc_generate(ASTNode *program, BytecodeBuffer *bb) {
    bc_init(bb);
    ASTNode *n = program->members;
    while (n) {
        if (n->type == AST_COMPONENT_DECL || n->type == AST_PAGE_DECL) {
            bc_emit(bb, OP_RENDER_START);
            /* find render block */
            ASTNode *m = n->members;
            while (m) {
                if (m->type == AST_RENDER_BLOCK) bc_gen_tmpl(bb, m->children);
                m = m->next;
            }
            bc_emit(bb, OP_RENDER_END);
        }
        n = n->next;
    }
    bc_emit(bb, OP_HALT);
}
