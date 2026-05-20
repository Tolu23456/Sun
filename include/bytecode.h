#ifndef SUN_BC_H
#define SUN_BC_H

#include <stdint.h>

typedef enum {
    OP_HALT,
    OP_LOAD_CONST,
    OP_LOAD_STATE,
    OP_STORE_STATE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_RENDER_START,
    OP_RENDER_END,
    OP_CREATE_EL,
    OP_SET_ATTR,
    OP_APPEND_CHILD,
    OP_CALL_COMPONENT,
    OP_NAVIGATE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_RETURN,
    /* UI Primitives as Components */
    OP_PRIMITIVE_DIV,
    OP_PRIMITIVE_BUTTON,
    OP_PRIMITIVE_SPAN,
    OP_PRIMITIVE_NAV,
    OP_PRIMITIVE_TEXT
} OpCode;

typedef struct {
    uint8_t *code;
    int size;
    int capacity;
} BytecodeBuffer;

#endif
