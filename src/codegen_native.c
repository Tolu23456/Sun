#define _POSIX_C_SOURCE 200809L
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *codegen_native_c(ASTNode *program) {
    CodegenCtx ctx;
    codegen_ctx_init(&ctx);

    emitln(&ctx, "#include <stdio.h>");
    emitln(&ctx, "#include <stdlib.h>");
    emitln(&ctx, "#include <stdbool.h>");
    emitln(&ctx, "");
    emitln(&ctx, "/* Sun Native Runtime - Minimal */");
    emitln(&ctx, "typedef struct { double value; } SunValue;");
    emitln(&ctx, "");

    /* Naive C generation for demo purposes */
    emitln(&ctx, "int main() {");
    ctx.indent++;
    emitln(&ctx, "printf(\"☀ Sun Native Server Running\\n\");");

    if (program && program->members) {
        ASTNode *n = program->members;
        while (n) {
            if (n->type == AST_COMPONENT_DECL || n->type == AST_PAGE_DECL) {
                if (n->str_val) emitf(&ctx, "  /* Component: %s */\n", n->str_val);
            }
            n = n->next;
        }
    }

    emitln(&ctx, "return 0;");
    ctx.indent--;
    emitln(&ctx, "}");

    char *result = strdup(ctx.buf);
    codegen_ctx_free(&ctx);
    return result;
}
