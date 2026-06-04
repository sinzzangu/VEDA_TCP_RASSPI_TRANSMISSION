#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "devices.h"

/* Global device instances */
DeviceThread led_dev;
DeviceThread buzzer_dev;
DeviceThread seg_dev;
DeviceThread cds_dev;
DeviceThread auto_light_dev;

/* Global function pointers set by devices_init */
ControlFns fns;

/* forward declaration */
static int is_http_fd(int fd);

/* Send a response string to a client fd */
void send_response(int client_fd, const char *msg) {
    if (client_fd < 0)
        return; /* sentinel (HTTP_FD -2), skip */
    if (is_http_fd(client_fd)) {
        int len = strlen(msg);
        char header[256];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %d\r\n"
                 "Access-Control-Allow-Origin: *\r\n"
                 "Connection: keep-alive\r\n"
                 "\r\n",
                 len);
        write(client_fd, header, strlen(header));
        write(client_fd, msg, len);
    } else {
        write(client_fd, msg, strlen(msg));
    }
}

/* ------------------------------------------------------------------ */
/*  HTTP fd registry                                                   */
/* ------------------------------------------------------------------ */
#define MAX_HTTP_FDS 16
static int http_fds[MAX_HTTP_FDS];
static int http_fd_count = 0;
static pthread_mutex_t http_fd_mutex = PTHREAD_MUTEX_INITIALIZER;

void register_http_fd(int fd) {
    pthread_mutex_lock(&http_fd_mutex);
    for (int i = 0; i < http_fd_count; i++)
        if (http_fds[i] == fd) {
            pthread_mutex_unlock(&http_fd_mutex);
            return;
        }
    if (http_fd_count < MAX_HTTP_FDS)
        http_fds[http_fd_count++] = fd;
    pthread_mutex_unlock(&http_fd_mutex);
}

void unregister_http_fd(int fd) {
    pthread_mutex_lock(&http_fd_mutex);
    for (int i = 0; i < http_fd_count; i++) {
        if (http_fds[i] == fd) {
            http_fds[i] = http_fds[--http_fd_count];
            break;
        }
    }
    pthread_mutex_unlock(&http_fd_mutex);
}

static int is_http_fd(int fd) {
    if (fd < 0)
        return 0;
    pthread_mutex_lock(&http_fd_mutex);
    int found = 0;
    for (int i = 0; i < http_fd_count; i++)
        if (http_fds[i] == fd) {
            found = 1;
            break;
        }
    pthread_mutex_unlock(&http_fd_mutex);
    return found;
}

static void init_device(DeviceThread *dev) {
    pthread_mutex_init(&dev->mutex, NULL);
    pthread_cond_init(&dev->cond, NULL);
    dev->busy = 0;
    dev->client_fd = -1;
}

void devices_init(ControlFns *control_fns) {
    fns = *control_fns;

    init_device(&led_dev);
    init_device(&buzzer_dev);
    init_device(&seg_dev);
    init_device(&cds_dev);
    init_device(&auto_light_dev);

    pthread_create(&led_dev.thread, NULL, led_thread_fn, NULL);
    pthread_create(&buzzer_dev.thread, NULL, buzzer_thread_fn, NULL);
    pthread_create(&seg_dev.thread, NULL, seg_thread_fn, NULL);
    pthread_create(&cds_dev.thread, NULL, cds_thread_fn, NULL);
    pthread_create(&auto_light_dev.thread, NULL, auto_light_thread_fn, NULL);

    pthread_detach(led_dev.thread);
    pthread_detach(buzzer_dev.thread);
    pthread_detach(seg_dev.thread);
    pthread_detach(cds_dev.thread);
    pthread_detach(auto_light_dev.thread);
}

/* Dispatch a command to the right device thread */
int devices_dispatch(int client_fd, Command *cmd) {
    DeviceThread *dev = NULL;

    /* special case: BUZZER OFF while song is playing */
    if (cmd->device == DEV_BUZZER && cmd->action == ACT_OFF) {
        pthread_mutex_lock(&buzzer_dev.mutex);
        if (buzzer_dev.busy) {
            if (buzzer_dev.client_fd == client_fd) {
                /* owner cancels — set flag, send 200 OK immediately */
                buzzer_dev.cancel = 1;
                pthread_mutex_unlock(&buzzer_dev.mutex);
                send_response(client_fd, "200 OK\n");
                return 0;
            } else {
                /* not the owner */
                pthread_mutex_unlock(&buzzer_dev.mutex);
                return -1;
            }
        }
        pthread_mutex_unlock(&buzzer_dev.mutex);
        /* not busy — fall through to normal dispatch */
    }

    switch (cmd->device) {
    case DEV_LED:
        dev = &led_dev;
        break;
    case DEV_BUZZER:
        dev = &buzzer_dev;
        break;
    case DEV_SEG:
        dev = &seg_dev;
        break;
    case DEV_CDS:
        if (cmd->action == ACT_AUTO)
            dev = &auto_light_dev;
        else
            dev = &cds_dev;
        break;
    default:
        return -2;
    }

    pthread_mutex_lock(&dev->mutex);
    printf("[DISPATCH] device=%d action=%d busy=%d\n", cmd->device, cmd->action, dev->busy); // ADD
    if (dev->busy) {
        pthread_mutex_unlock(&dev->mutex);
        return -1;
    }
    dev->cancel = 0; /* reset before queuing new command */
    dev->busy = 1;
    dev->client_fd = client_fd;
    dev->cmd = *cmd;
    pthread_cond_signal(&dev->cond);
    pthread_mutex_unlock(&dev->mutex);

    return 0;
}

/* Release all locks held by a disconnecting client */
void devices_release(int client_fd) {
    DeviceThread *devs[] = {&led_dev, &buzzer_dev, &seg_dev, &cds_dev, &auto_light_dev};

    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&devs[i]->mutex);
        if (devs[i]->client_fd == client_fd) {
            devs[i]->busy = 0;
            devs[i]->cancel = 1;
            devs[i]->client_fd = -1;
        }
        pthread_mutex_unlock(&devs[i]->mutex);
    }
}