#include <stdio.h>

#include "devices.h"

int auto_threshold = 500; /* default; overwritten by load_config in server.c */

void *auto_light_thread_fn(void *arg) {
    (void)arg;

    while (1) {
        /* wait for a CDS AUTO command */
        pthread_mutex_lock(&auto_light_dev.mutex);
        while (!auto_light_dev.busy)
            pthread_cond_wait(&auto_light_dev.cond, &auto_light_dev.mutex);

        int client_fd = auto_light_dev.client_fd;
        pthread_mutex_unlock(&auto_light_dev.mutex);

        /* read sensor directly — pure read, no hardware state change */
        int val = fns.read_cds();

        /* acquire LED lock manually — same as dispatch but we call fns directly
         * so led_thread doesn't send its own response to the client */
        pthread_mutex_lock(&led_dev.mutex);
        if (led_dev.busy) {
            pthread_mutex_unlock(&led_dev.mutex);
            send_response(client_fd, "503 BUSY\n");
        } else {
            led_dev.busy = 1;
            led_dev.client_fd = -1; /* internal — led_thread won't respond */
            pthread_mutex_unlock(&led_dev.mutex);

            /* control LED based on threshold */
            if (val > auto_threshold)
                fns.led_on();
            else
                fns.led_off();

            /* release LED lock */
            pthread_mutex_lock(&led_dev.mutex);
            led_dev.busy = 0;
            led_dev.client_fd = -1;
            pthread_mutex_unlock(&led_dev.mutex);

            /* send combined response to client */
            char response[64];
            snprintf(response, sizeof(response),
                     "200 OK\nvalue=%d\nled=%s\n",
                     val,
                     val > auto_threshold ? "ON" : "OFF");
            send_response(client_fd, response);
        }

        /* release auto_light lock */
        pthread_mutex_lock(&auto_light_dev.mutex);
        auto_light_dev.busy = 0;
        auto_light_dev.client_fd = -1;
        pthread_mutex_unlock(&auto_light_dev.mutex);
    }

    return NULL;
}