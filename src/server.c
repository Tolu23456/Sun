#define _POSIX_C_SOURCE 200809L
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const char *mime_html = "text/html; charset=utf-8";

static void send_response(int fd, int status, const char *status_text,
                          const char *content_type, const char *body) {
    char header[512];
    int blen = (int)strlen(body);
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n",
             status, status_text, content_type, blen);
    write(fd, header, strlen(header));
    write(fd, body, blen);
}

void server_serve(const char *html_content, int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(srv); return;
    }
    listen(srv, 8);

    printf("\033[1;33m☀  Sun dev server running at http://localhost:%d\033[0m\n", port);
    printf("   Press Ctrl+C to stop.\n\n");
    fflush(stdout);

    char req_buf[4096];
    for (;;) {
        int client = accept(srv, NULL, NULL);
        if (client < 0) continue;

        memset(req_buf, 0, sizeof(req_buf));
        read(client, req_buf, sizeof(req_buf) - 1);

        /* parse method + path */
        char method[8] = {0}, path[512] = {0};
        sscanf(req_buf, "%7s %511s", method, path);

        if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
            send_response(client, 200, "OK", mime_html, html_content);
        } else {
            send_response(client, 404, "Not Found", mime_html,
                          "<h1>404 - Not Found</h1>");
        }
        close(client);
    }
    close(srv);
}
