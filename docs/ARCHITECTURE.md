# Sun Architecture Design 2.0

Sun is an AOT-compiled language and runtime designed for the modern web.

## 1. Tooling Strategy: "The Unified Binary"
The `sun` binary (written in C) acts as a compiler, bundler, and runtime manager. It avoids the "dependency hell" of modern JS tooling by embedding everything needed to build and run applications.

## 2. Compilation Pipeline
1.  **Frontend (C):**
    - **Lexer:** Tokenizes `.sun` files, including embedded templates.
    - **Parser:** Constructs a Unified AST representing logic and UI.
    - **Semantic Analyzer:** Performs type inference and reactivity mapping.
2.  **Intermediate Layer:**
    - **Sun IR:** A high-level intermediate representation optimized for reactivity.
3.  **Backend Targets:**
    - **Web Backend:** Generates optimized ES6+ code and a minimal reactive runtime. Future support for WASM for compute-intensive blocks.
    - **Native Backend:** Generates C code, which is then compiled via Clang/GCC to native binaries for server-side execution.

## 3. Reactive Runtime (JS)
The Sun JS runtime is designed to be ultra-compact (<2KB).
- **Virtual DOM:** A lightweight VDOM implementation.
- **Client-Side Router:** Built-in SPA routing handling history API and path matching.
- **Batched Updates:** Reactivity is batched to ensure high frame rates.
- **Zero Dependencies:** The runtime is standalone.

## 4. Native Runtime & Cross-Platform (C)
For server-side and mobile execution, Sun provides:
- **Event Loop:** A custom epoll/kqueue-based event loop.
- **Native OS Bridge:** Lightweight FFI to access OS-native APIs (Notifications, Haptics, Storage) on Android (NDK), iOS, and Linux.
- **Memory Management:** Highly optimized manual management with optional reference counting for script-level objects.

## 5. .xsun Archive System
To simplify distribution, Sun introduces the `.xsun` format:
- **Structure:** A flat binary blob containing a file manifest followed by compressed file data (zstd/deflate).
- **Security:** Built-in integrity checks (SHA-256).
- **Portability:** A single `.xsun` file contains everything needed to reconstruct the project or run it in a Sun environment.

## 5. Directory Structure
- `/src`: Compiler source (C).
- `/include`: Header files.
- `/runtime`: Target-specific runtime implementations.
- `/docs`: Specification and architectural details.
