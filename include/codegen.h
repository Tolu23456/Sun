#ifndef SUN_CODEGEN_H
#define SUN_CODEGEN_H

#include "parser.h"

typedef struct {
    char  *buf;
    int    len;
    int    cap;
    int    indent;

    /* state variable names for this component (to prefix with this.state.) */
    char  *state_names[256];
    int    state_count;

    /* function names (to call as this.fn()) */
    char  *fn_names[256];
    int    fn_count;
} CodegenCtx;

void codegen_ctx_init(CodegenCtx *ctx);
void codegen_ctx_free(CodegenCtx *ctx);

/* Generate JavaScript bundle string from AST. Caller must free result. */
char *codegen_generate(ASTNode *program);

/* Generate full HTML page with embedded runtime + component JS. */
char *codegen_html_page(ASTNode *program, const char *title);

#endif
