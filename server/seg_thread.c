#include <stdio.h>
#include <unistd.h>

#include "devices.h"

void *seg_thread_fn(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&seg_dev.mutex);
        while (!seg_dev.busy)
            pthread_cond_wait(&seg_dev.cond, &seg_dev.mutex);

        Command cmd = seg_dev.cmd;
        int client_fd = seg_dev.client_fd;
        pthread_mutex_unlock(&seg_dev.mutex);
        printf("[SEG] action=%d value=%d\n", cmd.action, cmd.value);

        switch (cmd.action) {
        case ACT_DISPLAY:
            if (cmd.value >= 0 && cmd.value <= 9) {
                fns.seg_display(cmd.value);
                send_response(client_fd, "200 OK\n");
            } else {
                send_response(client_fd, "400 BAD REQUEST\n");
            }
            break;

        case ACT_CLEAR:
            fns.seg_clear();
            send_response(client_fd, "200 OK\n");
            break;

        case ACT_COUNT: {
            if (cmd.value < 0 || cmd.value > 9) {
                send_response(client_fd, "400 BAD REQUEST\n");
                break;
            }
            /* Lock buzzer for the full countdown duration */
            pthread_mutex_lock(&buzzer_dev.mutex);
            if (buzzer_dev.busy) {
                pthread_mutex_unlock(&buzzer_dev.mutex);
                send_response(client_fd, "503 BUSY\n");
                break;
            }
            buzzer_dev.busy = 1;
            buzzer_dev.client_fd = client_fd;
            pthread_mutex_unlock(&buzzer_dev.mutex);

            /* Countdown */
            for (int i = cmd.value; i >= 0; i--) {
                fns.seg_display(i);
                sleep(1);
            }

            /* Buzz at 0 */
            fns.buzzer_on();
            sleep(1);
            fns.buzzer_off();
            fns.seg_clear();

            /* Release buzzer */
            pthread_mutex_lock(&buzzer_dev.mutex);
            buzzer_dev.busy = 0;
            buzzer_dev.client_fd = -1;
            printf("[SEG] LOCKED buzzer for countdown\n"); // ADD
            pthread_mutex_unlock(&buzzer_dev.mutex);

            send_response(client_fd, "200 OK\n");
            break;
        }

        default:
            send_response(client_fd, "400 BAD REQUEST\n");
            break;
        }

        pthread_mutex_lock(&seg_dev.mutex);
        seg_dev.busy = 0;
        seg_dev.client_fd = -1;
        pthread_mutex_unlock(&seg_dev.mutex);
    }

    return NULL;
}