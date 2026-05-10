# Sun Language Project Configuration

## Project Overview
Sun is a new, C-based, AOT-compiled programming language designed as a high-performance runtime engine for web development. It focuses on simplicity, flexibility, and power, with a script-like syntax and a C-based runtime including an event loop for async I/O.

## Building and Running
- **Build:** Use the `make` command to compile the project.
- **Run:** After building, execute the binary: `./sun`.
- **Clean:** Use `make clean` to remove build artifacts.
- **Commands:** 
    - `sun ship [project_name]`: Scaffold a new Sun project.
    - `sun build [target]`: Build for specific platforms (android, ios, windows, linux, mac).
    - `sun clean`: Clean project artifacts.

## Development Conventions
- **Language:** C11.
- **Architecture:** AOT compilation (Frontend -> IR -> Backend).
- **Style:** Maintainable, performant C code with clear separation between the compiler frontend/backend and the runtime.
- **Compiler Flags:** `-Wall -Wextra -std=c11` are used to ensure code quality.
- **FFI/Low-level:** Native C interop is a core design pillar.
