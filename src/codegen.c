#define _POSIX_C_SOURCE 200809L
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

void buf_grow(CodegenCtx *ctx, int needed) {
    while (ctx->len + needed + 1 >= ctx->cap) {
        ctx->cap = ctx->cap ? ctx->cap * 2 : 4096;
        ctx->buf = realloc(ctx->buf, ctx->cap);
    }
}

void emit(CodegenCtx *ctx, const char *s) {
    int slen = (int)strlen(s);
    buf_grow(ctx, slen);
    memcpy(ctx->buf + ctx->len, s, slen);
    ctx->len += slen;
    ctx->buf[ctx->len] = '\0';
}

void emitf(CodegenCtx *ctx, const char *fmt, ...) {
    char tmp[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    emit(ctx, tmp);
}

void emit_indent(CodegenCtx *ctx) {
    for (int i = 0; i < ctx->indent; i++) emit(ctx, "  ");
}

void emitln(CodegenCtx *ctx, const char *s) {
    emit_indent(ctx);
    emit(ctx, s);
    emit(ctx, "\n");
}

void codegen_ctx_init(CodegenCtx *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->cap = 4096;
    ctx->buf = malloc(ctx->cap);
    ctx->buf[0] = '\0';
}

void codegen_ctx_free(CodegenCtx *ctx) {
    free(ctx->buf);
    for (int i = 0; i < ctx->state_count; i++) free(ctx->state_names[i]);
    for (int i = 0; i < ctx->fn_count;    i++) free(ctx->fn_names[i]);
}

static int is_state_var(CodegenCtx *ctx, const char *name) {
    for (int i = 0; i < ctx->state_count; i++)
        if (strcmp(ctx->state_names[i], name) == 0) return 1;
    return 0;
}

static int is_fn_name(CodegenCtx *ctx, const char *name) {
    for (int i = 0; i < ctx->fn_count; i++)
        if (strcmp(ctx->fn_names[i], name) == 0) return 1;
    return 0;
}

static void gen_expr(CodegenCtx *ctx, ASTNode *n) {
    if (!n) { emit(ctx, "undefined"); return; }
    switch (n->type) {
        case AST_NUMBER: {
            char tmp[64];
            if (n->num_val == (long)n->num_val) snprintf(tmp, sizeof(tmp), "%ld", (long)n->num_val);
            else snprintf(tmp, sizeof(tmp), "%g", n->num_val);
            emit(ctx, tmp);
            break;
        }
        case AST_STRING: emitf(ctx, "\"%s\"", n->str_val ? n->str_val : ""); break;
        case AST_BOOL: emit(ctx, n->bool_val ? "true" : "false"); break;
        case AST_NULL_LIT: emit(ctx, "null"); break;
        case AST_IDENT:
            if (n->str_val && is_state_var(ctx, n->str_val)) emitf(ctx, "_s.%s", n->str_val);
            else emit(ctx, n->str_val ? n->str_val : "undefined");
            break;
        case AST_BINARY:
            emit(ctx, "("); gen_expr(ctx, n->left); emitf(ctx, " %s ", n->str_val); gen_expr(ctx, n->right); emit(ctx, ")");
            break;
        case AST_UNARY:
            if (strcmp(n->str_val, "++") == 0 || strcmp(n->str_val, "--") == 0) {
                if (n->left && n->left->type == AST_IDENT && is_state_var(ctx, n->left->str_val)) {
                    const char *op = strcmp(n->str_val, "++") == 0 ? "+" : "-";
                    emitf(ctx, "this.setState({ %s: _s.%s %s 1 })", n->left->str_val, n->left->str_val, op);
                } else { gen_expr(ctx, n->left); emit(ctx, n->str_val); }
            } else { emit(ctx, n->str_val); gen_expr(ctx, n->left); }
            break;
        case AST_ASSIGN:
            if (n->left && n->left->type == AST_IDENT && is_state_var(ctx, n->left->str_val)) {
                const char *var = n->left->str_val;
                if (strcmp(n->str_val, "=") == 0) { emitf(ctx, "this.setState({ %s: ", var); gen_expr(ctx, n->right); emit(ctx, " })"); }
                else if (strcmp(n->str_val, "+=") == 0) { emitf(ctx, "this.setState({ %s: _s.%s + (", var, var); gen_expr(ctx, n->right); emit(ctx, ") })"); }
                else if (strcmp(n->str_val, "-=") == 0) { emitf(ctx, "this.setState({ %s: _s.%s - (", var, var); gen_expr(ctx, n->right); emit(ctx, ") })"); }
            } else { gen_expr(ctx, n->left); emitf(ctx, " %s ", n->str_val); gen_expr(ctx, n->right); }
            break;
        case AST_CALL:
            if (n->left && n->left->type == AST_IDENT && is_fn_name(ctx, n->left->str_val)) { emitf(ctx, "this.%s(", n->left->str_val); }
            else { gen_expr(ctx, n->left); emit(ctx, "("); }
            ASTNode *arg = n->args;
            while (arg) { gen_expr(ctx, arg); if (arg->next) emit(ctx, ", "); arg = arg->next; }
            emit(ctx, ")");
            break;
        case AST_MEMBER: gen_expr(ctx, n->left); emitf(ctx, ".%s", n->str_val); break;
        case AST_TERNARY: emit(ctx, "("); gen_expr(ctx, n->left); emit(ctx, " ? "); gen_expr(ctx, n->body); emit(ctx, " : "); gen_expr(ctx, n->else_br); emit(ctx, ")"); break;
        default: emit(ctx, "undefined");
    }
}

static void gen_stmt(CodegenCtx *ctx, ASTNode *n);

static void gen_block(CodegenCtx *ctx, ASTNode *block) {
    if (!block || block->type != AST_BLOCK) return;
    ASTNode *s = block->body;
    while (s) { gen_stmt(ctx, s); s = s->next; }
}

static void gen_stmt(CodegenCtx *ctx, ASTNode *n) {
    if (!n) return;
    switch (n->type) {
        case AST_BLOCK: gen_block(ctx, n); break;
        case AST_IF_STMT:
            emit_indent(ctx); emit(ctx, "if ("); gen_expr(ctx, n->left); emit(ctx, ") {\n");
            ctx->indent++; gen_block(ctx, n->body); ctx->indent--;
            emit_indent(ctx); emit(ctx, "}");
            if (n->else_br) { emit(ctx, " else {\n"); ctx->indent++; gen_block(ctx, n->else_br); ctx->indent--; emit_indent(ctx); emit(ctx, "}"); }
            emit(ctx, "\n");
            break;
        case AST_RETURN_STMT: emit_indent(ctx); emit(ctx, "return"); if (n->left) { emit(ctx, " "); gen_expr(ctx, n->left); } emit(ctx, ";\n"); break;
        case AST_VAR_DECL_STMT: emit_indent(ctx); emitf(ctx, "let %s", n->str_val); if (n->right) { emit(ctx, " = "); gen_expr(ctx, n->right); } emit(ctx, ";\n"); break;
        case AST_EXPR_STMT: emit_indent(ctx); gen_expr(ctx, n->left); emit(ctx, ";\n"); break;
        default: break;
    }
}

static void emit_tmpl_expr(CodegenCtx *ctx, const char *raw, int as_event) {
    if (as_event) emit(ctx, "() => { ");
    const char *p = raw;
    while (*p) {
        if (isspace((unsigned char)*p)) { emit(ctx, " "); p++; continue; }
        if (isalpha((unsigned char)*p) || *p == '_') {
            const char *start = p;
            while (isalnum((unsigned char)*p) || *p == '_') p++;
            int len = (int)(p - start);
            char name[256]; if (len >= 255) len = 255;
            memcpy(name, start, len); name[len] = '\0';
            const char *peek = p; while (isspace((unsigned char)*peek)) peek++;
            int is_inc = (peek[0] == '+' && peek[1] == '+');
            int is_dec = (peek[0] == '-' && peek[1] == '-');
            if (is_state_var(ctx, name)) {
                if (as_event && (is_inc || is_dec)) { emitf(ctx, "this.setState({ %s: _s.%s %s 1 })", name, name, is_inc ? "+" : "-"); p = peek + 2; }
                else { emitf(ctx, "_s.%s", name); }
            } else if (as_event && is_fn_name(ctx, name)) { emitf(ctx, "this.%s()", name); }
            else { emit(ctx, name); }
            continue;
        }
        if (isdigit((unsigned char)*p)) {
            const char *start = p; while (isdigit((unsigned char)*p) || *p == '.') p++;
            int len = (int)(p - start); char tmp[64]; if (len > 63) len = 63;
            memcpy(tmp, start, len); tmp[len] = '\0'; emit(ctx, tmp); continue;
        }
        if (*p == '"' || *p == '\'') {
            char q = *p++; emit(ctx, q == '"' ? "\"" : "'");
            while (*p && *p != q) { char cc[2] = { *p++, '\0' }; emit(ctx, cc); }
            if (*p) p++; emit(ctx, q == '"' ? "\"" : "'"); continue;
        }
        char cc[2] = { *p++, '\0' }; emit(ctx, cc);
    }
    if (as_event) emit(ctx, " }");
}

static int is_event_attr(const char *name) { return strncmp(name, "on", 2) == 0 && strlen(name) > 2; }

static void gen_template(CodegenCtx *ctx, ASTNode *n, int depth) {
    if (!n) return;
    switch (n->type) {
        case AST_ELEMENT:
            emit(ctx, "_h("); emitf(ctx, "\"%s\"", n->str_val); emit(ctx, ", {");
            ASTNode *attr = n->attrs;
            while (attr) {
                emitf(ctx, "%s: ", attr->str_val);
                if (attr->left) {
                    if (attr->left->type == AST_STRING) emitf(ctx, "\"%s\"", attr->left->str_val ? attr->left->str_val : "");
                    else if (attr->left->type == AST_INTERPOLATION) emit_tmpl_expr(ctx, attr->left->str_val, is_event_attr(attr->str_val));
                } else emit(ctx, "true");
                if (attr->next) emit(ctx, ", ");
                attr = attr->next;
            }
            emit(ctx, "}");
            ASTNode *child = n->children;
            while (child) {
                emit(ctx, ",\n"); for (int i = 0; i <= depth + 2; i++) emit(ctx, "  ");
                gen_template(ctx, child, depth + 1); child = child->next;
            }
            emit(ctx, ")"); break;
        case AST_TEXT:
            emit(ctx, "\""); const char *c = n->str_val;
            while (c && *c) {
                if (*c == '"') emit(ctx, "\\\""); else if (*c == '\\') emit(ctx, "\\\\");
                else if (*c == '\n' || *c == '\r') {} else { char cc[2] = { *c, '\0' }; emit(ctx, cc); }
                c++;
            }
            emit(ctx, "\""); break;
        case AST_INTERPOLATION: emit_tmpl_expr(ctx, n->str_val, 0); break;
        default: emit(ctx, "null");
    }
}

void gen_component(CodegenCtx *ctx, ASTNode *comp) {
    ctx->state_count = 0; ctx->fn_count = 0;
    ASTNode *m = comp->members;
    while (m) {
        if (m->type == AST_STATE_DECL && ctx->state_count < 255) ctx->state_names[ctx->state_count++] = strdup(m->str_val);
        if (m->type == AST_FN_DECL && ctx->fn_count < 255) ctx->fn_names[ctx->fn_count++] = strdup(m->str_val);
        m = m->next;
    }
    emitf(ctx, "class %s extends SunComponent {\n", comp->str_val);
    ctx->indent++; emitln(ctx, "constructor() {");
    ctx->indent++; emitln(ctx, "super();"); emitln(ctx, "this.state = {");
    ctx->indent++; m = comp->members;
    while (m) {
        if (m->type == AST_STATE_DECL) {
            emit_indent(ctx); emitf(ctx, "%s: ", m->str_val);
            int saved = ctx->state_count; ctx->state_count = 0; gen_expr(ctx, m->left); ctx->state_count = saved; emit(ctx, ",\n");
        }
        m = m->next;
    }
    ctx->indent--; emitln(ctx, "};"); ctx->indent--; emitln(ctx, "}");
    m = comp->members;
    while (m) {
        if (m->type == AST_FN_DECL) {
            emit(ctx, "\n"); emit_indent(ctx); emitf(ctx, "%s(", m->str_val);
            ASTNode *param = m->params; while (param) { emit(ctx, param->str_val); if (param->next) emit(ctx, ", "); param = param->next; }
            emit(ctx, ") {\n"); ctx->indent++; emitln(ctx, "const _s = this.state;");
            if (m->body) {
                if (m->body->type == AST_BLOCK) gen_block(ctx, m->body);
                else { emit_indent(ctx); emit(ctx, "return "); gen_expr(ctx, m->body); emit(ctx, ";\n"); }
            }
            ctx->indent--; emitln(ctx, "}");
        }
        m = m->next;
    }
    m = comp->members;
    while (m) {
        if (m->type == AST_RENDER_BLOCK) {
            emit(ctx, "\n"); emitln(ctx, "render() {");
            ctx->indent++; emitln(ctx, "const _s = this.state;"); emitln(ctx, "const _h = Sun.h;");
            emit_indent(ctx); emit(ctx, "return "); if (m->children) gen_template(ctx, m->children, 0); else emit(ctx, "_h('div', {})");
            emit(ctx, ";\n"); ctx->indent--; emitln(ctx, "}");
        }
        m = m->next;
    }
    ctx->indent--; emitln(ctx, "}"); emit(ctx, "\n");
}

char *codegen_generate(ASTNode *program) {
    CodegenCtx ctx; codegen_ctx_init(&ctx);
    ASTNode *node = program->members;
    while (node) {
        if (node->type == AST_COMPONENT_DECL) gen_component(&ctx, node);
        else if (node->type == AST_PAGE_DECL) {
            ASTNode comp_node = *node; comp_node.type = AST_COMPONENT_DECL;
            gen_component(&ctx, &comp_node);
            emitf(&ctx, "Sun.page(\"%s\", %s);\n", node->left ? node->left->str_val : "/", node->str_val);
        } else if (node->type == AST_MOUNT_CALL) {
            emitf(&ctx, "Sun.mount(%s, \"%s\");\n", node->str_val, node->left ? node->left->str_val : "#app");
        }
        node = node->next;
    }
    char *result = strdup(ctx.buf); codegen_ctx_free(&ctx); return result;
}

static const char *SUN_RUNTIME_JS =
  "const Sun = (() => {\n"
  "  class SunComponent {\n"
  "    constructor() {\n"
  "      this.state = {};\n"
  "      this._el = null;\n"
  "      this._mounted = false;\n"
  "    }\n"
  "    setState(updates) {\n"
  "      Object.assign(this.state, updates);\n"
  "      if (this._mounted) this._reconcile();\n"
  "    }\n"
  "    _reconcile() {\n"
  "      if (!this._el || !this._el.parentNode) return;\n"
  "      const newEl = Sun._createDOM(this.render(), this);\n"
  "      this._el.parentNode.replaceChild(newEl, this._el);\n"
  "      this._el = newEl;\n"
  "    }\n"
  "    render() { return Sun.h('div', {}); }\n"
  "  }\n"
  "  function h(tag, attrs, ...children) {\n"
  "    return { tag, attrs: attrs || {}, children: children.flat(Infinity).filter(c => c !== null && c !== undefined) };\n"
  "  }\n"
  "  function _createDOM(vnode, instance) {\n"
  "    if (typeof vnode === 'string' || typeof vnode === 'number' || typeof vnode === 'boolean') {\n"
  "      return document.createTextNode(String(vnode));\n"
  "    }\n"
  "    if (!vnode || !vnode.tag) return document.createTextNode('');\n"
  "    if (vnode.tag[0] === vnode.tag[0].toUpperCase()) {\n"
  "      const CompClass = window[vnode.tag];\n"
  "      if (CompClass) {\n"
  "        const inst = new CompClass();\n"
  "        return _createDOM(inst.render(), inst);\n"
  "      }\n"
  "    }\n"
  "    const el = document.createElement(vnode.tag);\n"
  "    if (vnode.attrs) {\n"
  "      for (const [k, v] of Object.entries(vnode.attrs)) {\n"
  "        if (k.startsWith('on') && typeof v === 'function') {\n"
  "          el.addEventListener(k.slice(2).toLowerCase(), v);\n"
  "        } else if (v !== null && v !== undefined && v !== false) {\n"
  "          el.setAttribute(k, String(v));\n"
  "        }\n"
  "      }\n"
  "    }\n"
  "    for (const child of (vnode.children || [])) {\n"
  "      el.appendChild(_createDOM(child, instance));\n"
  "    }\n"
  "    return el;\n"
  "  }\n"
  "  const routes = [];\n"
  "  function page(path, ComponentClass) {\n"
  "    routes.push({ path, ComponentClass });\n"
  "  }\n"
  "  function _matchRoute() {\n"
  "    const path = window.location.pathname || '/';\n"
  "    const route = routes.find(r => r.path === path) || routes[0];\n"
  "    if (route) {\n"
  "      const container = document.querySelector(Sun._selector || '#app');\n"
  "      if (!container) return;\n"
  "      const instance = new route.ComponentClass();\n"
  "      const el = _createDOM(instance.render(), instance);\n"
  "      container.innerHTML = '';\n"
  "      container.appendChild(el);\n"
  "      instance._el = el;\n"
  "      instance._mounted = true;\n"
  "    }\n"
  "  }\n"
  "  function mount(ComponentClass, selector) {\n"
  "    Sun._selector = selector;\n"
  "    const run = () => {\n"
  "      if (routes.length > 0) {\n"
  "        _matchRoute();\n"
  "        window.onpopstate = _matchRoute;\n"
  "      } else {\n"
  "        const container = document.querySelector(selector);\n"
  "        if (!container) return;\n"
  "        const instance = new ComponentClass();\n"
  "        const el = _createDOM(instance.render(), instance);\n"
  "        container.innerHTML = '';\n"
  "        container.appendChild(el);\n"
  "        instance._el = el;\n"
  "        instance._mounted = true;\n"
  "      }\n"
  "    };\n"
  "    if (document.readyState === 'loading') {\n"
  "      document.addEventListener('DOMContentLoaded', run);\n"
  "    } else {\n"
  "      run();\n"
  "    }\n"
  "  }\n"
  "  function navigate(path) {\n"
  "    window.history.pushState({}, '', path);\n"
  "    _matchRoute();\n"
  "  }\n"
  "  return { h, mount, page, navigate, _createDOM, Component: SunComponent };\n"
  "})();\n";

static const char *SUN_RUNTIME_CLASSES =
  "class SunComponent extends Sun.Component {}\n";
char *codegen_html_page(ASTNode *program, const char *title) {
    char *component_js = codegen_generate(program);
    CodegenCtx ctx; codegen_ctx_init(&ctx);
    emit(&ctx, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n  <meta charset=\"UTF-8\" />\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n");
    emitf(&ctx, "  <title>%s</title>\n", title ? title : "Sun App");
    emit(&ctx, "  <style>\n");

    /* Global styles from style blocks */
    ASTNode *n = program->members;
    while (n) {
        if (n->type == AST_COMPONENT_DECL || n->type == AST_PAGE_DECL) {
            ASTNode *m = n->members;
            while (m) {
                if (m->type == AST_STYLE_BLOCK && m->str_val) {
                    emit(&ctx, "    /* From component: ");
                    emit(&ctx, n->str_val);
                    emit(&ctx, " */\n");
                    emit(&ctx, m->str_val);
                    emit(&ctx, "\n");
                }
                m = m->next;
            }
        }
        n = n->next;
    }

    emit(&ctx,
"    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }\n"
"    body { font-family: -apple-system, sans-serif; background: #0f0f0f; color: #f0f0f0; min-height: 100vh; display: flex; align-items: center; justify-content: center; }\n"
"    #app { width: 100%; max-width: 900px; padding: 2rem; }\n"
"    .sun-app { display: flex; flex-direction: column; align-items: center; gap: 2rem; }\n"
"    .sun-hero h1 { font-size: 3.5rem; background: linear-gradient(135deg, #f5a623 0%, #f76b1c 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }\n"
"    .sun-card { background: #1a1a1a; border: 1px solid #2a2a2a; border-radius: 12px; padding: 1.5rem; }\n"
"    .count-display { font-size: 5rem; font-weight: 900; background: linear-gradient(135deg, #f5a623 0%, #f76b1c 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }\n"
"    .btn { border: none; cursor: pointer; font-size: 1rem; border-radius: 8px; padding: 0.6rem 1.4rem; }\n"
"    .btn-primary { background: linear-gradient(135deg, #f5a623, #f76b1c); color: #000; }\n"
"    .btn-secondary { background: #2a2a2a; color: #f0f0f0; }\n"
"    .sun-badge { background: #1a1a1a; border: 1px solid #2a2a2a; border-radius: 999px; padding: 0.25rem 0.75rem; color: #f5a623; }\n"
"  </style>\n</head>\n<body>\n  <div id=\"app\"></div>\n  <script>\n");
    emit(&ctx, SUN_RUNTIME_JS);
    emit(&ctx, SUN_RUNTIME_CLASSES); emit(&ctx, "\n// -- compiled from .sun source --\n");
    emit(&ctx, component_js); emit(&ctx, "  </script>\n</body>\n</html>\n");
    char *result = strdup(ctx.buf); codegen_ctx_free(&ctx); free(component_js); return result;
}
