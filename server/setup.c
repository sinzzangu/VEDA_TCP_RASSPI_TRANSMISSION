#include <arpa/inet.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "devices.h"
#include "setup.h"

/* ------------------------------------------------------------------ */
/*  Library loading                                                    */
/* ------------------------------------------------------------------ */
static void *link_library(void) {
    void *handle = dlopen("./libcontrol.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    dlerror(); /* clear error */
    return handle;
}

static void *get_function(void *handle, const char *name) {
    void *fn = dlsym(handle, name);
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "dlsym error: %s\n", error);
        exit(EXIT_FAILURE);
    }
    return fn;
}

/* ------------------------------------------------------------------ */
/*  server_setup                                                       */
/* ------------------------------------------------------------------ */
int server_setup(Config *cfg) {
    /* load shared library */
    void *handle = link_library();

    /* resolve lifecycle functions */
    int (*control_init)(void) = get_function(handle, "control_init");

    /* resolve device function pointers */
    ControlFns ctrl_fns = {
        .led_on = get_function(handle, "led_on"),
        .led_off = get_function(handle, "led_off"),
        .led_brightness = get_function(handle, "led_brightness"),
        .buzzer_on = get_function(handle, "buzzer_on"),
        .buzzer_off = get_function(handle, "buzzer_off"),
        .buzzer_tone = get_function(handle, "buzzer_tone"),
        .seg_display = get_function(handle, "seg_display"),
        .seg_clear = get_function(handle, "seg_clear"),
        .read_cds = get_function(handle, "read_cds"),
    };

    /* initialise hardware */
    if (control_init() != 0) {
        fprintf(stderr, "!![SETUP] control_init failed\n");
        return -1;
    }

    /* start device threads */
    devices_init(&ctrl_fns);

    /* set auto-light threshold from config */
    auto_threshold = cfg->threshold;

    /* socket setup */
    int server_fd;
    struct sockaddr_in server_address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("!![SETUP] SOCKET ERROR");
        return -1;
    }

    /* allow reuse of port immediately after server restart */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(cfg->port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("!![SETUP] BIND ERROR");
        return -1;
    }

    if (listen(server_fd, 10) == -1) {
        perror("!![SETUP] LISTEN ERROR");
        return -1;
    }

    printf("[SETUP] Server ready on port %d\n", cfg->port);
    return server_fd;
}