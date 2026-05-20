#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>

void sun_vm_execute(BytecodeBuffer *bb) {
    int ip = 0;
    double stack[256];
    int sp = 0;

    printf("  [VM] Starting execution... (GPU: %s)\n", bb->use_gpu ? "ENABLED" : "OFF");
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
            case OP_GUI_WINDOW: printf("  [VM] Creating Native GUI Window...\n"); break;
            case OP_GUI_RECT: printf("  [VM] Drawing Rect %s\n", bb->use_gpu ? "[GPU ACCELERATED]" : "[Software]"); break;
            case OP_GPU_SYNC: if (bb->use_gpu) printf("  [VM] GPU Buffer Sync\n"); break;
            case OP_FFI_CALL: printf("  [VM] FFI: Calling External C Function...\n"); break;
            default:
                break;
        }
    }
}
