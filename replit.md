# Sun Language

Sun is an AOT-compiled, C-based web programming language designed as a high-performance, reactive runtime for modern web development. It compiles `.sun` component files to optimized HTML + JavaScript bundles.

## Project Structure

```
sun/
├── src/
│   ├── main.c       — CLI (ship, build, serve, clean)
│   ├── lexer.c      — Full tokenizer for Sun syntax
│   ├── parser.c     — Recursive descent parser → AST
│   ├── codegen.c    — AST → JavaScript code generator + HTML page emitter
│   ├── server.c     — Built-in HTTP dev server
│   └── sun.c        — Core utilities (file I/O, errors)
├── include/
│   ├── sun.h        — Core types and error handling
│   ├── lexer.h      — Token types, Lexer struct
│   ├── parser.h     — AST node types, Parser struct
│   ├── codegen.h    — Code generator API
│   └── server.h     — Dev server API
├── docs/
│   ├── ARCHITECTURE.md
│   └── SPEC.md
└── Makefile
```

## Building

```bash
make
```

## CLI Commands

```bash
./sun ship <name>            # Scaffold a new Sun project with a demo component
./sun build <path>           # Compile .sun sources to dist/index.html
./sun serve <path> [-p port] # Compile and serve on localhost (default: 3000)
./sun clean <path>           # Remove dist/ build artifacts
./sun version                # Show version
```

## Sun Language Syntax

Sun uses a component-based, reactive syntax:

```sun
component Counter {
  // Reactive state
  state count = 0;
  state name  = "World";

  // Functions (mutations to state auto-trigger re-render)
  fn increment() {
    count = count + 1;
  }

  fn decrement() {
    if (count > 0) {
      count = count - 1;
    }
  }

  // JSX-like template
  render {
    <div class="app">
      <h1>Hello, {name}!</h1>
      <p>Count: {count}</p>
      <button onclick={decrement}>-</button>
      <button onclick={increment}>+</button>
    </div>
  }
}

mount(App, "#app");
```

## Compilation Pipeline

```
.sun source
    │
    ▼
Lexer  → Tokens (keywords, identifiers, operators, template markers)
    │
    ▼
Parser → AST (component/state/fn/render nodes, expression tree, template tree)
    │
    ▼
Codegen → JavaScript class extending SunComponent
    │       + embedded Sun reactive runtime
    ▼
dist/index.html  (fully self-contained, no build deps)
```

## Architecture

- **Language:** C11
- **Compiler:** GCC with `-Wall -Wextra -std=c11 -O2`
- **Parsing:** Recursive descent with 2-token lookahead
- **Template:** Raw-source character-level scanner (avoids lexer drift)
- **Output:** Standalone HTML page with embedded Sun runtime JS
- **Runtime:** Tiny virtual-DOM reactive system (no external deps)

## User Preferences

- Keep C11 standard and existing Makefile structure
- One binary — no Node.js, npm, or external tools required
