# Sun Language

Sun is an AOT-compiled, C-based programming language designed to be a high-performance, script-like runtime engine for modern web development.

## Core Pillars
- **Simplicity:** Minimal, clean syntax inspired by modern JavaScript.
- **Performance:** AOT (Ahead-of-Time) compilation to machine code.
- **Flexibility:** C-based runtime for easy FFI and low-level control.

## Getting Started

### Building
```bash
make
```

### Usage
Sun provides a CLI for managing projects:

- **Scaffold a new project:** 
  ```bash
  ./sun ship [project_name]
  ```
- **Build your project:**
  ```bash
  ./sun build [android|ios|windows|linux|mac]
  ```
- **Clean artifacts:**
  ```bash
  ./sun clean
  ```

## Development
- **Language:** C11
- **Architecture:** Frontend (Lexer/Parser) -> IR -> Backend (Compiler) -> Runtime (Event Loop).
- **Documentation:** See `docs/ARCHITECTURE.md` for deeper design insights.
