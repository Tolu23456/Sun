#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ── helpers ────────────────────────────────────────────────────── */

static ASTNode *make_node(ASTNodeType type, int line) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    n->type = type;
    n->line = line;
    return n;
}

static char *tok_str(Token t) {
    char *s = malloc(t.length + 1);
    memcpy(s, t.start, t.length);
    s[t.length] = '\0';
    return s;
}

void parser_init(Parser *p, Lexer *lexer, SunErrors *errors) {
    p->lexer  = lexer;
    p->errors = errors;
    p->had_error = 0;
    p->current = lexer_next_token(lexer);
    p->peek    = lexer_next_token(lexer);
}

static Token advance(Parser *p) {
    Token prev  = p->current;
    p->current  = p->peek;
    p->peek     = lexer_next_token(p->lexer);
    return prev;
}

static int check(Parser *p, TokenType t)  { return p->current.type == t; }
static int match(Parser *p, TokenType t)  { if (check(p, t)) { advance(p); return 1; } return 0; }

static Token expect(Parser *p, TokenType t, const char *msg) {
    if (check(p, t)) return advance(p);
    char err[256];
    snprintf(err, sizeof(err), "Line %d: Expected %s, got '%.*s' (%s)",
             p->current.line, msg,
             p->current.length, p->current.start,
             token_type_name(p->current.type));
    sun_error(p->errors, "%s", err);
    p->had_error = 1;
    return p->current;
}

/* ── forward declarations ───────────────────────────────────────── */
static ASTNode *parse_stmt(Parser *p);
static ASTNode *parse_expr(Parser *p);
static ASTNode *parse_assign(Parser *p);
/* ── free ───────────────────────────────────────────────────────── */

void ast_free(ASTNode *n) {
    if (!n) return;
    ast_free(n->left);
    ast_free(n->right);
    ast_free(n->body);
    ast_free(n->else_br);
    ast_free(n->args);
    ast_free(n->params);
    ast_free(n->attrs);
    ast_free(n->children);
    ast_free(n->members);
    ast_free(n->next);
    free(n->str_val);
    free(n);
}

/* ── expressions ────────────────────────────────────────────────── */

static ASTNode *parse_primary(Parser *p) {
    int line = p->current.line;

    if (check(p, TOKEN_NUMBER)) {
        Token t = advance(p);
        ASTNode *n = make_node(AST_NUMBER, line);
        char *s = tok_str(t);
        n->num_val = atof(s);
        free(s);
        return n;
    }
    if (check(p, TOKEN_STRING)) {
        Token t = advance(p);
        ASTNode *n = make_node(AST_STRING, line);
        /* strip quotes */
        int len = t.length >= 2 ? t.length - 2 : 0;
        n->str_val = malloc(len + 1);
        memcpy(n->str_val, t.start + 1, len);
        n->str_val[len] = '\0';
        return n;
    }
    if (check(p, TOKEN_TRUE)) {
        advance(p);
        ASTNode *n = make_node(AST_BOOL, line);
        n->bool_val = 1;
        return n;
    }
    if (check(p, TOKEN_FALSE)) {
        advance(p);
        ASTNode *n = make_node(AST_BOOL, line);
        n->bool_val = 0;
        return n;
    }
    if (check(p, TOKEN_NULL_KW)) {
        advance(p);
        return make_node(AST_NULL_LIT, line);
    }
    if (check(p, TOKEN_IDENTIFIER)) {
        Token t = advance(p);
        ASTNode *n = make_node(AST_IDENT, line);
        n->str_val = tok_str(t);
        return n;
    }
    if (match(p, TOKEN_LPAREN)) {
        ASTNode *inner = parse_expr(p);
        expect(p, TOKEN_RPAREN, ")");
        return inner;
    }
    if (check(p, TOKEN_NOT) || check(p, TOKEN_MINUS)) {
        Token op = advance(p);
        ASTNode *n = make_node(AST_UNARY, line);
        n->str_val = tok_str(op);
        n->left    = parse_primary(p);
        return n;
    }
    /* unknown */
    Token t = advance(p);
    (void)t;
    return make_node(AST_NULL_LIT, line);
}

static ASTNode *parse_postfix(Parser *p) {
    ASTNode *n = parse_primary(p);
    for (;;) {
        if (check(p, TOKEN_DOT) && p->peek.type == TOKEN_IDENTIFIER) {
            advance(p);
            Token field = advance(p);
            ASTNode *m = make_node(AST_MEMBER, field.line);
            m->left    = n;
            m->str_val = tok_str(field);
            n = m;
            continue;
        }
        if (check(p, TOKEN_LPAREN)) {
            advance(p);
            ASTNode *call = make_node(AST_CALL, n->line);
            call->left   = n;
            ASTNode *tail = NULL;
            while (!check(p, TOKEN_RPAREN) && !check(p, TOKEN_EOF)) {
                ASTNode *arg = parse_assign(p);
                if (call->args == NULL) { call->args = arg; tail = arg; }
                else                   { tail->next = arg; tail = arg; }
                if (!match(p, TOKEN_COMMA)) break;
            }
            expect(p, TOKEN_RPAREN, ")");
            n = call;
            continue;
        }
        if (check(p, TOKEN_PLUS_PLUS) || check(p, TOKEN_MINUS_MINUS)) {
            Token op = advance(p);
            ASTNode *u = make_node(AST_UNARY, op.line);
            u->str_val = tok_str(op);
            u->left = n;
            n = u;
            continue;
        }
        break;
    }
    return n;
}

static ASTNode *parse_mul(Parser *p) {
    ASTNode *n = parse_postfix(p);
    while (check(p, TOKEN_STAR) || check(p, TOKEN_SLASH) || check(p, TOKEN_PERCENT)) {
        Token op = advance(p);
        ASTNode *b = make_node(AST_BINARY, op.line);
        b->str_val = tok_str(op);
        b->left    = n;
        b->right   = parse_postfix(p);
        n = b;
    }
    return n;
}

static ASTNode *parse_add(Parser *p) {
    ASTNode *n = parse_mul(p);
    while (check(p, TOKEN_PLUS) || check(p, TOKEN_MINUS)) {
        Token op = advance(p);
        ASTNode *b = make_node(AST_BINARY, op.line);
        b->str_val = tok_str(op);
        b->left    = n;
        b->right   = parse_mul(p);
        n = b;
    }
    return n;
}

static ASTNode *parse_cmp(Parser *p) {
    ASTNode *n = parse_add(p);
    while (check(p, TOKEN_LT)  || check(p, TOKEN_GT) ||
           check(p, TOKEN_LTE) || check(p, TOKEN_GTE)) {
        Token op = advance(p);
        ASTNode *b = make_node(AST_BINARY, op.line);
        b->str_val = tok_str(op);
        b->left    = n;
        b->right   = parse_add(p);
        n = b;
    }
    return n;
}

static ASTNode *parse_eq(Parser *p) {
    ASTNode *n = parse_cmp(p);
    while (check(p, TOKEN_EQ) || check(p, TOKEN_NEQ)) {
        Token op = advance(p);
        ASTNode *b = make_node(AST_BINARY, op.line);
        b->str_val = tok_str(op);
        b->left    = n;
        b->right   = parse_cmp(p);
        n = b;
    }
    return n;
}

static ASTNode *parse_logic(Parser *p) {
    ASTNode *n = parse_eq(p);
    while (check(p, TOKEN_AND) || check(p, TOKEN_OR)) {
        Token op = advance(p);
        ASTNode *b = make_node(AST_BINARY, op.line);
        b->str_val = tok_str(op);
        b->left    = n;
        b->right   = parse_eq(p);
        n = b;
    }
    return n;
}

static ASTNode *parse_ternary(Parser *p) {
    ASTNode *n = parse_logic(p);
    if (match(p, TOKEN_QUESTION)) {
        ASTNode *t = make_node(AST_TERNARY, n->line);
        t->left    = n;
        t->body    = parse_assign(p);
        expect(p, TOKEN_COLON, ":");
        t->else_br = parse_assign(p);
        return t;
    }
    return n;
}

static ASTNode *parse_assign(Parser *p) {
    ASTNode *n = parse_ternary(p);
    if (check(p, TOKEN_ASSIGN) || check(p, TOKEN_PLUS_ASSIGN) || check(p, TOKEN_MINUS_ASSIGN)) {
        Token op = advance(p);
        ASTNode *a = make_node(AST_ASSIGN, op.line);
        a->str_val = tok_str(op);
        a->left    = n;
        a->right   = parse_assign(p);
        return a;
    }
    return n;
}

static ASTNode *parse_expr(Parser *p) { return parse_assign(p); }

/* ── statements ─────────────────────────────────────────────────── */

static ASTNode *parse_block(Parser *p) {
    expect(p, TOKEN_LBRACE, "{");
    ASTNode *block = make_node(AST_BLOCK, p->current.line);
    ASTNode *tail  = NULL;
    while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF)) {
        ASTNode *s = parse_stmt(p);
        if (!s) break;
        if (!block->body) { block->body = s; tail = s; }
        else              { tail->next  = s; tail = s; }
    }
    expect(p, TOKEN_RBRACE, "}");
    return block;
}

static ASTNode *parse_stmt(Parser *p) {
    int line = p->current.line;

    if (check(p, TOKEN_IF)) {
        advance(p);
        expect(p, TOKEN_LPAREN, "(");
        ASTNode *cond = parse_expr(p);
        expect(p, TOKEN_RPAREN, ")");
        ASTNode *n   = make_node(AST_IF_STMT, line);
        n->left      = cond;
        if (check(p, TOKEN_LBRACE)) {
            n->body = parse_block(p);
        } else {
            n->body = parse_stmt(p);
        }
        if (match(p, TOKEN_ELSE)) {
            if (check(p, TOKEN_LBRACE)) {
                n->else_br = parse_block(p);
            } else {
                n->else_br = parse_stmt(p);
            }
        }
        return n;
    }
    if (check(p, TOKEN_RETURN)) {
        advance(p);
        ASTNode *n = make_node(AST_RETURN_STMT, line);
        if (!check(p, TOKEN_SEMICOLON) && !check(p, TOKEN_RBRACE))
            n->left = parse_expr(p);
        match(p, TOKEN_SEMICOLON);
        return n;
    }
    if (check(p, TOKEN_LET) || check(p, TOKEN_CONST) || check(p, TOKEN_VAR)) {
        advance(p);
        Token name = expect(p, TOKEN_IDENTIFIER, "variable name");
        ASTNode *n = make_node(AST_VAR_DECL_STMT, line);
        n->str_val = tok_str(name);
        if (match(p, TOKEN_ASSIGN)) n->right = parse_expr(p);
        match(p, TOKEN_SEMICOLON);
        return n;
    }
    /* expression statement (assignment, call, ++/--, etc.) */
    ASTNode *e = parse_expr(p);
    if (e->type == AST_IDENT && match(p, TOKEN_PLUS_PLUS)) {
        ASTNode *u = make_node(AST_UNARY, line);
        u->str_val = strdup("++");
        u->left = e;
        e = u;
    } else if (e->type == AST_IDENT && match(p, TOKEN_MINUS_MINUS)) {
        ASTNode *u = make_node(AST_UNARY, line);
        u->str_val = strdup("--");
        u->left = e;
        e = u;
    }
    match(p, TOKEN_SEMICOLON);
    ASTNode *s = make_node(AST_EXPR_STMT, line);
    s->left    = e;
    return s;
}

/* ── template parser ────────────────────────────────────────────── */

static void skip_template_ws(const char **cur) {
    while (**cur == ' ' || **cur == '\t' || **cur == '\n' || **cur == '\r')
        (*cur)++;
}

static char *read_tag_name(const char **cur) {
    const char *start = *cur;
    while (isalnum((unsigned char)**cur) || **cur == '-' || **cur == '_')
        (*cur)++;
    int len = (int)(*cur - start);
    if (len == 0) return NULL;
    char *s = malloc(len + 1);
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

static char *read_attr_value_str(const char **cur) {
    /* eat opening quote */
    char quote = **cur; (*cur)++;
    const char *start = *cur;
    while (**cur && **cur != quote) (*cur)++;
    int len = (int)(*cur - start);
    if (**cur == quote) (*cur)++;
    char *s = malloc(len + 1);
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

/*
 * parse_template_expr_text: read raw source until '}', return as string.
 */
static char *read_until_close_brace(const char **cur) {
    const char *start = *cur;
    int depth = 1;
    while (**cur) {
        if (**cur == '{') depth++;
        else if (**cur == '}') { depth--; if (depth == 0) break; }
        (*cur)++;
    }
    int len = (int)(*cur - start);
    char *s = malloc(len + 1);
    memcpy(s, start, len);
    s[len] = '\0';
    if (**cur == '}') (*cur)++;
    return s;
}

/* parse one template node starting at *cur; returns NULL if nothing left */
static ASTNode *parse_tmpl_at(const char **cur, int *line) {
    skip_template_ws(cur);
    if (!**cur || **cur == '<') {
        /* peek: if it's </, stop */
        if (**cur == '<' && *(*cur+1) == '/') return NULL;
        if (!**cur) return NULL;
    }

    /* { expression } interpolation */
    if (**cur == '{') {
        (*cur)++;
        char *expr_text = read_until_close_brace(cur);
        ASTNode *n = make_node(AST_INTERPOLATION, *line);
        n->str_val = expr_text;
        return n;
    }

    /* @event shorthand */
    if (**cur == '@') {
        (*cur)++;
        char *tag = read_tag_name(cur);
        ASTNode *attr = make_node(AST_ATTR, *line);
        /* prepend 'on' to the event name */
        char *on_event = malloc(strlen(tag) + 3);
        strcpy(on_event, "on");
        strcat(on_event, tag);
        attr->str_val = on_event;
        free(tag);

        if (**cur == '=') {
            (*cur)++;
            if (**cur == '{') {
                (*cur)++;
                attr->left = make_node(AST_INTERPOLATION, *line);
                attr->left->str_val = read_until_close_brace(cur);
            }
        }
        return attr;
    }

    /* <tag ...> element */
    if (**cur == '<' && *(*cur+1) != '/') {
        (*cur)++; /* eat < */
        skip_template_ws(cur);
        char *tag = read_tag_name(cur);
        if (!tag) return NULL;

        ASTNode *el = make_node(AST_ELEMENT, *line);
        el->str_val = tag;

        /* parse attributes */
        ASTNode *attr_tail = NULL;
        for (;;) {
            skip_template_ws(cur);
            if (!**cur || **cur == '>' || (**cur == '/' && *(*cur+1) == '>')) break;
            /* attribute name / shorthand */
            ASTNode *attr = NULL;
            if (**cur == '.') {
                (*cur)++;
                char *name = read_tag_name(cur);
                attr = make_node(AST_ATTR, *line);
                attr->str_val = strdup("class");
                attr->left = make_node(AST_STRING, *line);
                attr->left->str_val = name;
            } else if (**cur == '@') {
                (*cur)++;
                char *name = read_tag_name(cur);
                attr = make_node(AST_ATTR, *line);
                char *on_event = malloc(strlen(name) + 3);
                strcpy(on_event, "on");
                strcat(on_event, name);
                attr->str_val = on_event;
                free(name);
            } else {
                const char *astart = *cur;
                while (**cur && **cur != '=' && **cur != '>' && **cur != ' ' &&
                       **cur != '\t' && **cur != '\n' && **cur != '/' && **cur != '{')
                    (*cur)++;
                int alen = (int)(*cur - astart);
                if (alen == 0) break;
                char *aname = malloc(alen + 1);
                memcpy(aname, astart, alen);
                aname[alen] = '\0';

                attr = make_node(AST_ATTR, *line);
                attr->str_val = aname;
            }

            if (**cur == '=') {
                (*cur)++;
                if (**cur == '"' || **cur == '\'') {
                    /* string value */
                    attr->left = make_node(AST_STRING, *line);
                    attr->left->str_val = read_attr_value_str(cur);
                } else if (**cur == '{') {
                    (*cur)++;
                    char *expr_text = read_until_close_brace(cur);
                    attr->left = make_node(AST_INTERPOLATION, *line);
                    attr->left->str_val = expr_text;
                }
            }

            if (!el->attrs) { el->attrs = attr; attr_tail = attr; }
            else            { attr_tail->next = attr; attr_tail = attr; }
        }

        /* self-closing? */
        if (**cur == '/' && *(*cur+1) == '>') {
            (*cur) += 2;
            return el;
        }
        if (**cur == '>') (*cur)++;

        /* parse children */
        ASTNode *child_tail = NULL;
        for (;;) {
            skip_template_ws(cur);
            if (!**cur) break;
            if (**cur == '<' && *(*cur+1) == '/') break;
            ASTNode *child = parse_tmpl_at(cur, line);
            if (!child) break;
            if (!el->children) { el->children = child; child_tail = child; }
            else               { child_tail->next = child; child_tail = child; }
        }

        /* closing </tag> */
        if (**cur == '<' && *(*cur+1) == '/') {
            *cur += 2;
            while (**cur && **cur != '>') (*cur)++;
            if (**cur == '>') (*cur)++;
        }
        return el;
    }

    /* text content */
    const char *start = *cur;
    while (**cur && **cur != '<' && **cur != '{' && **cur != '}') {
        if (**cur == '\n') (*line)++;
        (*cur)++;
    }
    int len = (int)(*cur - start);
    if (len == 0) return NULL;

    /* trim pure-whitespace text nodes */
    int all_ws = 1;
    for (int i = 0; i < len; i++) {
        if (start[i] != ' ' && start[i] != '\t' && start[i] != '\n' && start[i] != '\r') {
            all_ws = 0; break;
        }
    }
    if (all_ws) return NULL;

    ASTNode *t = make_node(AST_TEXT, *line);
    t->str_val = malloc(len + 1);
    memcpy(t->str_val, start, len);
    t->str_val[len] = '\0';
    return t;
}

static ASTNode *parse_render_block(Parser *p) {
    /* p->current is TOKEN_LBRACE. Save the raw pointer right after '{'.
       This must happen BEFORE advance() because advance() will call
       lexer_next_token and drift the lexer past the template content. */
    const char *after_brace = p->current.start + 1;
    int start_line = p->current.line;

    /* Consume the LBRACE from the parser token stream (has lexer side-effect,
       but we will reset the lexer below, so that's fine). */
    advance(p);

    /* Parse the template directly from raw source position. */
    const char *cur  = after_brace;
    int         tline = start_line;

    ASTNode *render = make_node(AST_RENDER_BLOCK, start_line);

    skip_template_ws(&cur);
    ASTNode *root = parse_tmpl_at(&cur, &tline);
    render->children = root;

    /* Move past any trailing whitespace to find the render block's closing } */
    skip_template_ws(&cur);
    if (*cur == '}') cur++;

    /* Reset the lexer to the position right after the closing '}' */
    p->lexer->current = cur;
    p->lexer->line    = tline;

    /* Resync both parser lookahead tokens from the corrected lexer position */
    p->current = lexer_next_token(p->lexer);
    p->peek    = lexer_next_token(p->lexer);

    return render;
}

/* ── component members ──────────────────────────────────────────── */

static ASTNode *parse_state_decl(Parser *p) {
    int line = p->current.line;
    expect(p, TOKEN_STATE, "state");
    Token name = expect(p, TOKEN_IDENTIFIER, "state name");
    expect(p, TOKEN_ASSIGN, "=");
    ASTNode *value = parse_expr(p);
    match(p, TOKEN_SEMICOLON);
    ASTNode *n = make_node(AST_STATE_DECL, line);
    n->str_val = tok_str(name);
    n->left    = value;
    return n;
}

static ASTNode *parse_fn_decl(Parser *p) {
    int line = p->current.line;
    expect(p, TOKEN_FN, "fn");
    Token name = expect(p, TOKEN_IDENTIFIER, "function name");
    expect(p, TOKEN_LPAREN, "(");
    ASTNode *param_head = NULL, *param_tail = NULL;
    while (!check(p, TOKEN_RPAREN) && !check(p, TOKEN_EOF)) {
        Token pname = expect(p, TOKEN_IDENTIFIER, "param name");
        ASTNode *param = make_node(AST_FN_PARAM, pname.line);
        param->str_val = tok_str(pname);
        if (!param_head) { param_head = param; param_tail = param; }
        else             { param_tail->next = param; param_tail = param; }
        if (!match(p, TOKEN_COMMA)) break;
    }
    expect(p, TOKEN_RPAREN, ")");

    ASTNode *n    = make_node(AST_FN_DECL, line);
    n->str_val    = tok_str(name);
    n->params     = param_head;

    if (match(p, TOKEN_ARROW)) {
        n->body = parse_expr(p);
    } else {
        n->body = parse_block(p);
    }
    return n;
}

static ASTNode *parse_component(Parser *p) {
    int line = p->current.line;
    expect(p, TOKEN_COMPONENT, "component");
    Token name = expect(p, TOKEN_IDENTIFIER, "component name");
    expect(p, TOKEN_LBRACE, "{");

    ASTNode *comp = make_node(AST_COMPONENT_DECL, line);
    comp->str_val = tok_str(name);
    ASTNode *tail = NULL;

    while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF)) {
        ASTNode *member = NULL;
        if (check(p, TOKEN_STATE))  member = parse_state_decl(p);
        else if (check(p, TOKEN_FN)) member = parse_fn_decl(p);
        else if (check(p, TOKEN_STYLE)) {
            advance(p);
            member = make_node(AST_STYLE_BLOCK, p->current.line);
            expect(p, TOKEN_LBRACE, "{");
            const char *start = p->current.start;
            int depth = 1;
            while (depth > 0 && !check(p, TOKEN_EOF)) {
                if (check(p, TOKEN_LBRACE)) depth++;
                else if (check(p, TOKEN_RBRACE)) depth--;
                if (depth > 0) advance(p);
            }
            int len = (int)(p->current.start - start);
            member->str_val = malloc(len + 1);
            memcpy(member->str_val, start, len);
            member->str_val[len] = '\0';
            expect(p, TOKEN_RBRACE, "}");
        } else if (check(p, TOKEN_RENDER)) {
            advance(p);
            member = parse_render_block(p);
        } else {
            advance(p); /* skip unknown token */
            continue;
        }
        if (member) {
            if (!comp->members) { comp->members = member; tail = member; }
            else                { tail->next    = member; tail = member; }
        }
    }
    expect(p, TOKEN_RBRACE, "}");
    return comp;
}

/* ── top-level parse ────────────────────────────────────────────── */

ASTNode *parser_parse(Parser *p) {
    ASTNode *program = make_node(AST_PROGRAM, 1);
    ASTNode *tail    = NULL;

    while (!check(p, TOKEN_EOF)) {
        ASTNode *node = NULL;

        if (check(p, TOKEN_COMPONENT)) {
            node = parse_component(p);
        } else if (check(p, TOKEN_PAGE)) {
            int line = p->current.line;
            advance(p);
            Token name = expect(p, TOKEN_IDENTIFIER, "page name");
            Token path = expect(p, TOKEN_STRING, "page path");

            node = make_node(AST_PAGE_DECL, line);
            node->str_val = tok_str(name);
            node->left = make_node(AST_STRING, line);
            int slen = path.length >= 2 ? path.length - 2 : 0;
            node->left->str_val = malloc(slen + 1);
            memcpy(node->left->str_val, path.start + 1, slen);
            node->left->str_val[slen] = '\0';

            expect(p, TOKEN_LBRACE, "{");
            ASTNode *comp_tail = NULL;
            while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF)) {
                ASTNode *member = NULL;
                if (check(p, TOKEN_STATE))  member = parse_state_decl(p);
                else if (check(p, TOKEN_FN)) member = parse_fn_decl(p);
                else if (check(p, TOKEN_STYLE)) {
                    advance(p);
                    member = make_node(AST_STYLE_BLOCK, p->current.line);
                    expect(p, TOKEN_LBRACE, "{");
                    const char *start = p->current.start;
                    int depth = 1;
                    while (depth > 0 && !check(p, TOKEN_EOF)) {
                        if (check(p, TOKEN_LBRACE)) depth++;
                        else if (check(p, TOKEN_RBRACE)) depth--;
                        if (depth > 0) advance(p);
                    }
                    int len = (int)(p->current.start - start);
                    member->str_val = malloc(len + 1);
                    memcpy(member->str_val, start, len);
                    member->str_val[len] = '\0';
                    expect(p, TOKEN_RBRACE, "}");
                }
                else if (check(p, TOKEN_RENDER)) { advance(p); member = parse_render_block(p); }
                else { advance(p); continue; }
                if (member) {
                    if (!node->members) { node->members = member; comp_tail = member; }
                    else                { comp_tail->next = member; comp_tail = member; }
                }
            }
            expect(p, TOKEN_RBRACE, "}");
        } else if (check(p, TOKEN_MOUNT)) {
            int line = p->current.line;
            advance(p);
            expect(p, TOKEN_LPAREN, "(");
            Token comp = expect(p, TOKEN_IDENTIFIER, "component name");
            expect(p, TOKEN_COMMA, ",");
            Token selector_tok = expect(p, TOKEN_STRING, "selector string");
            expect(p, TOKEN_RPAREN, ")");
            match(p, TOKEN_SEMICOLON);
            node = make_node(AST_MOUNT_CALL, line);
            node->str_val = tok_str(comp);
            /* strip quotes from selector */
            int slen = selector_tok.length >= 2 ? selector_tok.length - 2 : 0;
            node->left = make_node(AST_STRING, line);
            node->left->str_val = malloc(slen + 1);
            memcpy(node->left->str_val, selector_tok.start + 1, slen);
            node->left->str_val[slen] = '\0';
        } else if (check(p, TOKEN_IMPORT)) {
            int line = p->current.line;
            advance(p);
            expect(p, TOKEN_LBRACE, "{");
            Token comp_name = expect(p, TOKEN_IDENTIFIER, "component name");
            expect(p, TOKEN_RBRACE, "}");
            expect(p, TOKEN_FROM, "from");
            Token path_tok = expect(p, TOKEN_STRING, "import path");
            match(p, TOKEN_SEMICOLON);

            node = make_node(AST_IMPORT_DECL, line);
            node->str_val = tok_str(comp_name);
            int slen = path_tok.length >= 2 ? path_tok.length - 2 : 0;
            node->left = make_node(AST_STRING, line);
            node->left->str_val = malloc(slen + 1);
            memcpy(node->left->str_val, path_tok.start + 1, slen);
            node->left->str_val[slen] = '\0';
        } else {
            advance(p);
            continue;
        }

        if (node) {
            if (!program->members) { program->members = node; tail = node; }
            else                   { tail->next = node;        tail = node; }
        }
    }
    return program;
}
