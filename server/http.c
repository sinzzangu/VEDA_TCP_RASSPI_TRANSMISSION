#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "devices.h"
#include "http.h"

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */
static void serve_html(int fd) {
    FILE *f = fopen("index.html", "r");
    if (!f) {
        const char *err =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 21\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "index.html not found\n";
        write(fd, err, strlen(err));
        return;
    }
    static char html[16384];
    int n = fread(html, 1, sizeof(html) - 1, f);
    fclose(f);
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=utf-8\r\n"
             "Content-Length: %d\r\n"
             "Connection: keep-alive\r\n"
             "\r\n",
             n);
    write(fd, header, strlen(header));
    write(fd, html, n);
}

static void http_ok(int fd) {
    const char *r =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    write(fd, r, strlen(r));
}

/* ------------------------------------------------------------------ */
/*  http_parse                                                         */
/* ------------------------------------------------------------------ */
int http_parse(int fd, const char *request, char *cmd_out, int cmd_size) {
    /* CORS preflight */
    if (strncmp(request, "OPTIONS", 7) == 0) {
        const char *r =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
            "Content-Length: 0\r\n"
            "Connection: keep-alive\r\n"
            "\r\n";
        write(fd, r, strlen(r));
        return 0;
    }

    /* favicon */
    if (strncmp(request, "GET /favicon.ico", 16) == 0) {
        const char *r = "HTTP/1.1 204 No Content\r\nConnection: keep-alive\r\n\r\n";
        write(fd, r, strlen(r));
        return 0;
    }

    /* serve web UI */
    if (strncmp(request, "GET /", 5) == 0) {
        serve_html(fd);
        return 0;
    }

    /* tab closed — stop owned devices and close connection */
    if (strncmp(request, "POST /disconnect", 16) == 0) {
        if (buzzer_dev.client_fd == fd)
            fns.buzzer_off();
        if (led_dev.client_fd == fd)
            fns.led_off();
        if (seg_dev.client_fd == fd)
            fns.seg_clear();
        devices_release(fd);
        http_ok(fd);
        shutdown(fd, SHUT_RDWR);
        return -1;
    }

    /* device command — extract body */
    if (strncmp(request, "POST /command", 13) == 0) {
        const char *body = strstr(request, "\r\n\r\n");
        if (!body)
            return 0;
        body += 4;

        strncpy(cmd_out, body, cmd_size - 1);
        cmd_out[cmd_size - 1] = '\0';
        int l = strlen(cmd_out);
        while (l > 0 && (cmd_out[l - 1] == '\r' || cmd_out[l - 1] == '\n' || cmd_out[l - 1] == ' '))
            cmd_out[--l] = '\0';

        return 1; /* cmd_out is ready to dispatch */
    }

    /* unknown request */
    const char *r =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    write(fd, r, strlen(r));
    return 0;
}