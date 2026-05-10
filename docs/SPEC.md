# Sun Language Specification (Finalized Decisions)

Sun is a high-performance, all-in-one web runtime engine inspired by the philosophy of Bun.

## 1. Syntax & Core Features
- **Type System:** Dynamic Typing (inferred at runtime).
- **Async Model:** `async/await` syntax.
- **Memory Management:** Automated (Reference Counting).

## 2. Web Integration & Batteries-Included
- **Runtime:** C-based engine with an embedded high-performance event loop.
- **Native SQLite:** Built-in `sun:sqlite` module for zero-overhead local storage.
- **Web API Interop:** First-class support for `fetch`, `HTTP` server, and `WebSocket`.
- **All-in-One Tooling:** Sun binary includes a bundler, test runner, and package manager.
- **WASM Support:** Native execution target for browser portability.

## 3. Implementation Roadmap
1. **Frontend:** Lexer (Done), Parser (In Progress), AST.
2. **Intermediate:** IR Generator.
3. **Backend:** Code Generator (LLVM/C-transpilation).
4. **Runtime:** Event loop, async/await runtime, memory manager, `sun:sqlite` integration.
