# Sun Architecture Design 2.0

Sun is an AOT-compiled language and runtime designed for the modern web.

## 1. Tooling Strategy: "The Unified Binary"
The `sun` binary (written in C) acts as a compiler, bundler, and runtime manager. It avoids the "dependency hell" of modern JS tooling by embedding everything needed to build and run applications.

## 2. Compilation Pipeline: The SunBC Path
1.  **Frontend (C):**
    - **Lexer:** Tokenizes `.sun` files.
    - **Parser:** Constructs a Unified AST where logic and components are first-class nodes.
2.  **Backend (Bytecode Generator):**
    - **bcgen.c:** Translates AST nodes into **SunBC** instructions. Components are compiled into "Component Definition" chunks within the bytecode.
3.  **Deployment:**
    - The bytecode is packaged into a `.xsun` archive along with assets.

## 3. The Sun VM
Instead of transpiling to JS/HTML, the Sun server runs the **Sun VM**.
- **Bytecode Execution:** High-speed instruction fetching and dispatch.
- **Component Orchestration:** The VM manages the component tree, lifecycle, and state.
- **Rendering:** The VM outputs a compact binary representation of the UI tree, which a minimal client (thin-client) or the VM's native UI layer renders.

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
