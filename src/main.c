#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "sun.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "server.h"
#include "archive.h"

/* ── ANSI colors ────────────────────────────────────────────────── */
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define ORANGE  "\033[38;5;214m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

/* ── help ───────────────────────────────────────────────────────── */

static void print_banner(void) {
    printf("\n");
    printf(ORANGE BOLD "  ☀  Sun " RESET BOLD "v%s" RESET "\n", SUN_VERSION);
    printf(DIM "  A high-performance web programming language\n" RESET);
    printf("\n");
}

static void print_help(void) {
    print_banner();
    printf(BOLD "  Usage:" RESET "  sun <command> [options]\n\n");
    printf(BOLD "  Commands:\n" RESET);
    printf("    " CYAN "ship" RESET "   <name>          Scaffold a new Sun project\n");
    printf("    " CYAN "build" RESET "  [path] [target]  Compile .sun sources\n");
    printf("    " CYAN "serve" RESET "  [path] [-p port]  Build and serve on localhost\n");
    printf("    " CYAN "pack" RESET "   <dir> <file>     Bundle codebase into .xsun\n");
    printf("    " CYAN "unpack" RESET " <file> <dir>     Restore codebase from .xsun\n");
    printf("    " CYAN "clean" RESET "  [path]           Remove dist/ artifacts\n");
    printf("    " CYAN "version" RESET "                 Show Sun version\n");
    printf("\n");
    printf(BOLD "  Examples:\n" RESET);
    printf("    sun ship my-app\n");
    printf("    sun serve my-app\n");
    printf("    sun build my-app\n");
    printf("\n");
}

/* ── scaffold (sun ship) ────────────────────────────────────────── */

static const char *DEMO_MAIN_SUN =
"// Welcome to Sun — a reactive web programming language.\n"
"// Components are the building blocks of your UI.\n"
"\n"
"component App {\n"
"  state count = 0;\n"
"  state title = \"Sun\";\n"
"\n"
"  fn increment() {\n"
"    count = count + 1;\n"
"  }\n"
"\n"
"  fn decrement() {\n"
"    if (count > 0) {\n"
"      count = count - 1;\n"
"    }\n"
"  }\n"
"\n"
"  fn reset() {\n"
"    count = 0;\n"
"  }\n"
"\n"
"  render {\n"
"    <div class=\"sun-app\">\n"
"      <div class=\"sun-hero\">\n"
"        <h1>Welcome to {title}</h1>\n"
"        <p>A reactive, component-based web programming language.</p>\n"
"      </div>\n"
"      <div class=\"sun-cards\">\n"
"        <div class=\"sun-card\">\n"
"          <h3>Components</h3>\n"
"          <p>Build UIs with clean, composable components and reactive state.</p>\n"
"        </div>\n"
"        <div class=\"sun-card\">\n"
"          <h3>Fast</h3>\n"
"          <p>Compiled ahead-of-time by a native C compiler for zero-overhead output.</p>\n"
"        </div>\n"
"        <div class=\"sun-card\">\n"
"          <h3>Simple</h3>\n"
"          <p>Minimal syntax inspired by modern JavaScript — easy to read and write.</p>\n"
"        </div>\n"
"      </div>\n"
"      <div class=\"sun-counter\">\n"
"        <h2>Interactive Counter</h2>\n"
"        <div class=\"count-display\">{count}</div>\n"
"        <div class=\"btn-row\">\n"
"          <button class=\"btn btn-danger\" onclick={decrement}>-</button>\n"
"          <button class=\"btn btn-secondary\" onclick={reset}>Reset</button>\n"
"          <button class=\"btn btn-primary\" onclick={increment}>+</button>\n"
"        </div>\n"
"      </div>\n"
"      <div class=\"sun-footer\">\n"
"        <span class=\"sun-badge\">sun v0.1.0</span>\n"
"      </div>\n"
"    </div>\n"
"  }\n"
"}\n"
"\n"
"mount(App, \"#app\");\n";

static const char *DEMO_SUN_JSON_FMT =
"{\n"
"  \"name\": \"%s\",\n"
"  \"version\": \"0.1.0\",\n"
"  \"entry\": \"src/main.sun\",\n"
"  \"title\": \"%s\"\n"
"}\n";

static const char *DEMO_GITIGNORE =
"dist/\n"
"*.o\n";

static void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, RED "  error: cannot write %s\n" RESET, path); return; }
    fputs(content, f);
    fclose(f);
    printf(GREEN "  created" RESET "  %s\n", path);
}

static void scaffold_project(const char *name) {
    print_banner();
    printf(BOLD "  Scaffolding project: " ORANGE "%s\n\n" RESET, name);

    char path[SUN_MAX_PATH];

    if (mkdir(name, 0755) != 0) {
        if (errno == EEXIST)
            printf(DIM "  (directory already exists, adding files)\n" RESET);
        else {
            fprintf(stderr, RED "  error: cannot create directory '%s'\n" RESET, name);
            return;
        }
    }

    snprintf(path, sizeof(path), "%s/src", name);
    mkdir(path, 0755);

    snprintf(path, sizeof(path), "%s/dist", name);
    mkdir(path, 0755);

    /* src/main.sun */
    snprintf(path, sizeof(path), "%s/src/main.sun", name);
    write_file(path, DEMO_MAIN_SUN);

    /* sun.json */
    snprintf(path, sizeof(path), "%s/sun.json", name);
    {
        char content[512];
        snprintf(content, sizeof(content), DEMO_SUN_JSON_FMT, name, name);
        write_file(path, content);
    }

    /* .gitignore */
    snprintf(path, sizeof(path), "%s/.gitignore", name);
    write_file(path, DEMO_GITIGNORE);

    printf("\n" GREEN BOLD "  ✓ Done!" RESET "\n\n");
    printf("  Next steps:\n");
    printf("    " CYAN "cd %s\n" RESET, name);
    printf("    " CYAN "sun serve .\n" RESET);
    printf("\n");
}

/* ── read sun.json ──────────────────────────────────────────────── */

static void read_sun_json(const char *project_path, char *title_out, char *entry_out) {
    char json_path[SUN_MAX_PATH];
    snprintf(json_path, sizeof(json_path), "%s/sun.json", project_path);
    char *content = sun_read_file(json_path);
    if (!content) return;

    /* naive key extraction */
    const char *p;
    if ((p = strstr(content, "\"title\"")) != NULL) {
        p = strchr(p, ':'); if (p) p++;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '"') {
            p++;
            int i = 0;
            while (*p && *p != '"' && i < 255) title_out[i++] = *p++;
            title_out[i] = '\0';
        }
    }
    if ((p = strstr(content, "\"entry\"")) != NULL) {
        p = strchr(p, ':'); if (p) p++;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '"') {
            p++;
            int i = 0;
            while (*p && *p != '"' && i < 511) entry_out[i++] = *p++;
            entry_out[i] = '\0';
        }
    }
    free(content);
}

/* ── compile one .sun file → HTML ───────────────────────────────── */

static char *compile_sun_file(const char *src_path, const char *title) {
    char *source = sun_read_file(src_path);
    if (!source) {
        fprintf(stderr, RED "  error: cannot read '%s'\n" RESET, src_path);
        return NULL;
    }

    SunErrors errors;
    errors.count = 0;

    Lexer  lexer;
    Parser parser;
    lexer_init(&lexer, source);
    parser_init(&parser, &lexer, &errors);

    ASTNode *program = parser_parse(&parser);

    if (errors.count > 0) {
        fprintf(stderr, RED "\n  Compile errors in %s:\n" RESET, src_path);
        for (int i = 0; i < errors.count; i++)
            fprintf(stderr, "    %s\n", errors.messages[i]);
        ast_free(program);
        free(source);
        return NULL;
    }

    char *html = codegen_html_page(program, title);
    ast_free(program);
    free(source);
    return html;
}

/* ── sun build ──────────────────────────────────────────────────── */

static int cmd_build(const char *project_path, const char *target) {
    char entry[SUN_MAX_PATH]  = "src/main.sun";
    char title[256]           = "Sun App";
    read_sun_json(project_path, title, entry);

    char src_path[SUN_MAX_PATH];
    snprintf(src_path, sizeof(src_path), "%s/%s", project_path, entry);

    if (strcmp(target, "web") == 0) {
        printf(BOLD "  Building [web] " RESET "%s " DIM "→ dist/index.html\n" RESET, src_path);
        char *html = compile_sun_file(src_path, title);
        if (!html) return 1;
        char dist[SUN_MAX_PATH];
        snprintf(dist, sizeof(dist), "%s/dist", project_path);
        mkdir(dist, 0755);
        char out_path[SUN_MAX_PATH];
        snprintf(out_path, sizeof(out_path), "%s/dist/index.html", project_path);
        sun_write_file(out_path, html);
        printf(GREEN "  ✓ Built" RESET "   %s\n", out_path);
        free(html);
    } else {
        /* Native build: android, ios, linux */
        printf(BOLD "  Building [%s] " RESET "%s " DIM "→ dist/native/\n" RESET, target, src_path);
        char *source = sun_read_file(src_path);
        if (!source) return 1;
        Lexer lexer; SunErrors errs; errs.count = 0;
        lexer_init(&lexer, source);
        Parser parser;
        parser_init(&parser, &lexer, &errs);
        ASTNode *program = parser_parse(&parser);
        char *c_code = codegen_native_c(program);

        char dist[SUN_MAX_PATH];
        snprintf(dist, sizeof(dist), "%s/dist/native", project_path);
        sun_mkdir_p(dist);
        char out_path[SUN_MAX_PATH];
        snprintf(out_path, sizeof(out_path), "%s/dist/native/app.c", project_path);
        sun_write_file(out_path, c_code);

        printf(GREEN "  ✓ Generated" RESET " %s\n", out_path);
        printf(DIM "  (Use a native compiler for %s to finish the build)\n" RESET, target);

        free(c_code);
        ast_free(program);
        free(source);
    }

    return 0;
}

/* ── sun serve ──────────────────────────────────────────────────── */

static int cmd_serve(const char *project_path, int port) {
    char entry[SUN_MAX_PATH]  = "src/main.sun";
    char title[256]           = "Sun App";
    read_sun_json(project_path, title, entry);

    char src_path[SUN_MAX_PATH];
    snprintf(src_path, sizeof(src_path), "%s/%s", project_path, entry);

    printf(BOLD "\n  ☀  Sun serve\n" RESET);
    printf("  Compiling " CYAN "%s" RESET "...\n", src_path);

    char *html = compile_sun_file(src_path, title);
    if (!html) return 1;

    /* also write dist/index.html */
    char dist[SUN_MAX_PATH];
    snprintf(dist, sizeof(dist), "%s/dist", project_path);
    mkdir(dist, 0755);
    char out_path[SUN_MAX_PATH];
    snprintf(out_path, sizeof(out_path), "%s/dist/index.html", project_path);
    sun_write_file(out_path, html);

    printf(GREEN "  ✓ Compiled successfully\n" RESET);
    server_serve(html, port);
    free(html);
    return 0;
}

/* ── sun clean ──────────────────────────────────────────────────── */

static void cmd_clean(const char *project_path) {
    char dist[SUN_MAX_PATH];
    snprintf(dist, sizeof(dist), "%s/dist", project_path);
    char cmd[SUN_MAX_PATH + 16];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", dist);
    system(cmd);
    printf(GREEN "  ✓ Cleaned" RESET " %s/dist/\n", project_path);
}

/* ── main ───────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    if (argc < 2) { print_help(); return 0; }

    const char *cmd = argv[1];

    if (strcmp(cmd, "version") == 0 || strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
        printf("sun %s\n", SUN_VERSION);
        return 0;
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(cmd, "ship") == 0) {
        if (argc < 3) { fprintf(stderr, RED "  error: missing project name\n  Usage: sun ship <name>\n" RESET); return 1; }
        scaffold_project(argv[2]);
        return 0;
    }

    if (strcmp(cmd, "build") == 0) {
        const char *path = argc >= 3 ? argv[2] : ".";
        const char *target = argc >= 4 ? argv[3] : "web";
        print_banner();
        return cmd_build(path, target);
    }

    if (strcmp(cmd, "serve") == 0) {
        const char *path = ".";
        int port = 3000;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                port = atoi(argv[++i]);
            } else {
                path = argv[i];
            }
        }
        return cmd_serve(path, port);
    }

    if (strcmp(cmd, "pack") == 0) {
        if (argc < 4) { fprintf(stderr, RED "  error: sun pack <dir> <out.xsun>\n" RESET); return 1; }
        return sun_pack(argv[2], argv[3]);
    }

    if (strcmp(cmd, "unpack") == 0) {
        if (argc < 4) { fprintf(stderr, RED "  error: sun unpack <file.xsun> <out_dir>\n" RESET); return 1; }
        return sun_unpack(argv[2], argv[3]);
    }

    if (strcmp(cmd, "clean") == 0) {
        const char *path = argc >= 3 ? argv[2] : ".";
        cmd_clean(path);
        return 0;
    }

    fprintf(stderr, RED "  error: unknown command '%s'\n" RESET, cmd);
    print_help();
    return 1;
}
