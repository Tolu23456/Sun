#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>

void sun_vm_execute(BytecodeBuffer *bb) {
    int ip = 0;
    double stack[256];
    int sp = 0;

    printf("  [VM] Starting execution...\n");
    while (ip < bb->size) {
        uint8_t op = bb->code[ip++];
        switch (op) {
            case OP_HALT:
                printf("  [VM] Halted.\n");
                return;
            case OP_LOAD_CONST:
                stack[sp++] = bb->code[ip++];
                break;
            case OP_ADD: {
                double b = stack[--sp];
                double a = stack[--sp];
                stack[sp++] = a + b;
                break;
            }
            case OP_RENDER_START:
                printf("  [VM] Rendering component...\n");
                break;
            case OP_RENDER_END:
                printf("  [VM] Rendering finished.\n");
                break;
            case OP_PRIMITIVE_DIV: printf("  [VM] <div />\n"); break;
            case OP_PRIMITIVE_BUTTON: printf("  [VM] <button />\n"); break;
            case OP_PRIMITIVE_SPAN: printf("  [VM] <span />\n"); break;
            case OP_PRIMITIVE_NAV: printf("  [VM] <nav />\n"); break;
            case OP_PRIMITIVE_TEXT: printf("  [VM] \"text content\"\n"); break;
            default:
                break;
        }
    }
}
