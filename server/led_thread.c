#include <stdio.h>

#include "devices.h"

// led thread function
void *led_thread_fn(void *arg) {

    while (1) {
        // wait for the device lock
        pthread_mutex_lock(&led_dev.mutex);
        while (!led_dev.busy)
            pthread_cond_wait(&led_dev.cond, &led_dev.mutex);

        Command cmd = led_dev.cmd;
        int client_fd = led_dev.client_fd;
        pthread_mutex_unlock(&led_dev.mutex);

        switch (cmd.action) {
        case ACT_ON:
            fns.led_on();
            break;
        case ACT_OFF:
            fns.led_off();
            break;
        case ACT_BRIGHT:
            fns.led_brightness(cmd.value);
            break;
        default:
            if (client_fd >= 0)
                send_response(client_fd, "400 BAD REQUEST\n");
            break;
        }

        if (client_fd >= 0)
            send_response(client_fd, "200 OK\n");

        pthread_mutex_lock(&led_dev.mutex);
        led_dev.busy = 0;
        led_dev.client_fd = -1;
        pthread_mutex_unlock(&led_dev.mutex);
    }

    return NULL;
}