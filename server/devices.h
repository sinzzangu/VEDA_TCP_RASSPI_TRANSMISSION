#ifndef DEVICES_H
#define DEVICES_H

#include "protocol.h"
#include <pthread.h>

/* Function pointers loaded from libcontrol.so by server.c */
typedef struct {
    void (*led_on)(void);
    void (*led_off)(void);
    void (*led_brightness)(int);
    void (*buzzer_on)(void);
    void (*buzzer_off)(void);
    void (*buzzer_tone)(int);
    void (*seg_display)(int);
    void (*seg_clear)(void);
    int (*read_cds)(void);
} ControlFns;

/* One persistent thread per device */
typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int busy;      /* 1 = device is in use    */
    int cancel;    /* 1 = owner requested stop */
    int client_fd; /* fd to send response to  */
    Command cmd;   /* pending command         */
} DeviceThread;

/* Global device instances — defined in devices.c */
extern DeviceThread led_dev;
extern DeviceThread buzzer_dev;
extern DeviceThread seg_dev;
extern DeviceThread cds_dev;
extern DeviceThread auto_light_dev;

/* Global function pointers — defined in devices.c, set by devices_init */
extern ControlFns fns;

/* Threshold for auto light — defined in auto_light_thread.c, set by server.c */
extern int auto_threshold;

/* Thread functions — one per device .c file */
void *led_thread_fn(void *arg);
void *buzzer_thread_fn(void *arg);
void *seg_thread_fn(void *arg);
void *cds_thread_fn(void *arg);
void *auto_light_thread_fn(void *arg);

/* Send a response string to a client fd */
void send_response(int client_fd, const char *msg);

/* Register/unregister an fd as an HTTP client.
 * send_response wraps the message in HTTP headers for registered fds. */
void register_http_fd(int fd);
void unregister_http_fd(int fd);

/* Initialise all device threads. Call once after dlopen. */
void devices_init(ControlFns *control_fns);

/* Dispatch a command to the right device thread.
 * Returns  0 — command queued.
 * Returns -1 — device busy (caller sends 503 BUSY).
 * Returns -2 — invalid command for this device. */
int devices_dispatch(int client_fd, Command *cmd);

/* Release all locks held by client_fd. Call on client disconnect. */
void devices_release(int client_fd);

#endif /* DEVICES_H */