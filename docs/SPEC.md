# Sun Language 2.0 Specification

Sun is a high-performance, compact web programming language designed for speed and developer happiness.

## 1. Syntax Design: "The Compact Pillar"

Sun achieves maximum compactness by removing boilerplate and using intuitive sigils.

### 1.1 Variables and Types
Sun uses `let` for all variables. Types are optional (Gradual Typing).
```sun
let name = "Sun"           // Inferred string
let version: num = 2.0     // Explicit type
const pi = 3.14            // Immutable
```

### 1.2 Functions
Ultra-compact arrow syntax.
```sun
// TypeScript
const add = (a: number, b: number): number => a + b;

// Sun
fn add(a, b) => a + b
```

### 1.3 Components
Components are first-class citizens. Reactivity is built-in.
```sun
component Counter {
  state count = 0

  render {
    <div .card>
      <h1>Count: {count}</h1>
      <button @click={count++}>Increment</button>
    </div>
  }
}
```
*   `.class` shorthand for classes.
*   `@event` shorthand for event listeners.
*   Automatic reactivity: incrementing `count` triggers a re-render.

### 1.4 Async Operations
Top-level await and clean async syntax.
```sun
fn fetchData(url) {
  let resp = fetch(url).await
  return resp.json().await
}
```
Or prefix:
```sun
let data = await fetch(url)
```

### 1.5 SPA Routing
Navigation is a first-class citizen.
```sun
page Home "/" { ... }
page Profile "/user/:id" { ... }
```

### 1.6 Pure Sun Modules & Styling
No more external HTML/CSS. Everything is a `.sun` component.
```sun
import { Nav } from "./components/Nav.sun"

component Header {
  style {
    .header { background: #333; color: white; padding: 1rem; }
  }

  render {
    <header .header>
      <Nav />
      <h1>Sun Application</h1>
    </header>
  }
}
```

## 2. Runtime & Speed: "The Blazing Pillar"

### 2.1 SunBC & The Sun VM
Sun moves beyond transpilation. It uses a high-performance bytecode format (**SunBC**) and a custom virtual machine (**Sun VM**).
- **Target Server:** The server understands SunBC directly. It renders components into a stream that the client displays. No JS/HTML/CSS generation during compilation—just optimized bytecode.
- **Unified Components:** Everything is a component. Low-level primitives like `button`, `nav`, and `div` are intrinsic components within the VM.
- **Cross-Platform (Mobile/Desktop):** The Sun VM is ported to Linux, Android, and iOS, providing a consistent execution environment with native OS bindings.

### 2.2 Built-in Tooling
The `sun` binary is all you need:
- `sun build`: Compiles and bundles for web/native.
- `sun pack`: Bundles entire codebase into a `.xsun` binary archive.
- `sun unpack`: Restores a codebase from an `.xsun` file.
- `sun serve`: Fast dev server with SPA support and HMR.

### 2.3 Universal GUI & GPU Acceleration
Sun is no longer limited to the browser.
- **GPU Rendering:** Developers can enable blazing-fast hardware acceleration by setting `use_gpu = True` in their component or project config.
- **Universal FFI:** Sun components can call functions in C, Rust, or Python with zero-overhead FFI, making it compatible with any ecosystem.
- **Native GUI:** Primitives for Windowing, Canvas, and Input are built into the VM.

## 3. Developer Experience: "The Easy Pillar"

- **No Config:** Zero-setup projects. `sun ship` creates a ready-to-go app.
- **Single File:** Components, logic, and styles can live in a single `.sun` file.
- **Universal:** Same language for frontend and backend.

## 4. Comparison Table

| Feature | TypeScript / React | Sun 2.0 |
| :--- | :--- | :--- |
| Variable | `const x: number = 1;` | `let x = 1` |
| Function | `const f = (x: number) => x * 2;` | `fn f(x) => x * 2` |
| Component | `function App() { ... }` | `component App { ... }` |
| State | `const [c, setC] = useState(0);` | `state c = 0` |
| Event | `onClick={() => setC(c + 1)}` | `@click={c++}` |
| Async | `const data = await res.json();` | `let data = res.json().await` |
