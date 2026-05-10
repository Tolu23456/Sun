# Sun Architecture Design

Sun is an AOT-compiled language designed to be a high-performance, script-like runtime engine for modern web development.

## Core Pillars
- **Simplicity:** Clean, minimal syntax inspired by modern JS.
- **Performance:** AOT compilation for machine code output.
- **Flexibility:** C-based runtime for easy FFI and low-level control.

## Component Stack
1. **Frontend:**
    - Scanner/Lexer: Tokenizes the input source.
    - Parser: Builds an AST (Abstract Syntax Tree).
    - Semantic Analyzer: Ensures type safety and scope validation.
2. **Intermediate Layer:**
    - IR Generation: Converts AST to a machine-agnostic representation.
3. **Backend:**
    - Code Generator: Translates IR to C code or directly to LLVM IR.
    - Compiler: Uses system C compiler or LLVM to produce an executable.
4. **Runtime (C):**
    - Memory Management: Custom allocator or wrapper around `malloc/free`.
    - Event Loop: Asynchronous I/O handling (similar to libuv/Bun).
    - Standard Library: Web-focused built-ins (net, fs, etc.).
