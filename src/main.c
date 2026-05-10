#include <stdio.h>
#include <string.h>
#include "sun.h"

void print_help() {
    printf("Usage: sun <command> [arguments]\n");
    printf("Commands:\n");
    printf("  ship [project_name]   Scaffold a new Sun project\n");
    printf("  build [target]        Build for android, ios, windows, linux, or mac\n");
    printf("  clean                 Clean build artifacts\n");
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
        printf("Shipping project: %s\n", argv[2]);
        // Implement project scaffolding here
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
