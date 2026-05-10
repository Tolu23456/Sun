# Sun Language

Sun is an AOT-compiled, C-based programming language designed as a high-performance, script-like runtime engine for modern web development.

## Project Structure

- `src/` — C source files (lexer, parser, main CLI)
- `include/` — Header files
- `docs/` — Architecture and language specification docs
- `Makefile` — Build system
- `sun` — Compiled binary (after build)

## Building

```bash
make
```

This compiles all `.c` files in `src/` using GCC with C11 standard and outputs the `sun` binary.

## Usage

```bash
./sun ship [project_name]   # Scaffold a new Sun project
./sun build [target]        # Build for android, ios, windows, linux, or mac
./sun clean                 # Clean build artifacts
```

## Development

- **Language:** C11
- **Compiler:** GCC with `-Wall -Wextra -std=c11`
- **Architecture:** Lexer → Parser → IR → Backend (Code Generator) → Runtime

## User Preferences

- Keep C11 standard and existing Makefile structure
