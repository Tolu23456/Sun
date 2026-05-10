#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sun.h"

void print_help() {
    printf("Usage: sun <command> [arguments]\n");
    printf("Commands:\n");
    printf("  ship [project_name]   Scaffold a new Sun project\n");
    printf("  build [target]        Build for android, ios, windows, linux, or mac\n");
    printf("  clean                 Clean build artifacts\n");
}

void scaffold_project(const char *project_name) {
    printf("Scaffolding project: %s\n", project_name);

    // Create project directory
    if (mkdir(project_name, 0755) != 0) {
        printf("Error: Could not create directory '%s'\n", project_name);
        return;
    }

    char path[256];

    // Create src directory
    snprintf(path, sizeof(path), "%s/src", project_name);
    mkdir(path, 0755);

    // Create main.sun file
    snprintf(path, sizeof(path), "%s/src/main.sun", project_name);
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "// Welcome to Sun!\n\nfn main() {\n  log(\"Hello from %s!\");\n}\n", project_name);
        fclose(f);
    }

    // Create sun.json config
    snprintf(path, sizeof(path), "%s/sun.json", project_name);
    f = fopen(path, "w");
    if (f) {
        fprintf(f, "{\n  \"name\": \"%s\",\n  \"version\": \"0.1.0\"\n}\n", project_name);
        fclose(f);
    }

    printf("Successfully scaffolded '%s'.\n", project_name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "ship") == 0) {
        if (argc < 3) {
            printf("Error: Missing project name.\n");
            return 1;
        }
        scaffold_project(argv[2]);
    } else if (strcmp(argv[1], "build") == 0) {
        if (argc < 3) {
            printf("Error: Missing target platform.\n");
            return 1;
        }
        printf("Building for: %s\n", argv[2]);
        // Implement build logic here
    } else if (strcmp(argv[1], "clean") == 0) {
        printf("Cleaning build artifacts...\n");
        // Implement clean logic here
    } else {
        printf("Unknown command: %s\n", argv[1]);
        print_help();
        return 1;
    }

    return 0;
}
