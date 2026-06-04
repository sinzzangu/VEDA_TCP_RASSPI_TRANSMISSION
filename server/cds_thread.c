#include <stdio.h>

#include "devices.h"

void *cds_thread_fn(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&cds_dev.mutex);
        while (!cds_dev.busy)
            pthread_cond_wait(&cds_dev.cond, &cds_dev.mutex);

        Command cmd = cds_dev.cmd;
        int client_fd = cds_dev.client_fd;
        pthread_mutex_unlock(&cds_dev.mutex);

        switch (cmd.action) {
        case ACT_READ: {
            int val = fns.read_cds();
            char response[64];
            snprintf(response, sizeof(response), "200 OK\nvalue=%d\n", val);
            send_response(client_fd, response);
            break;
        }
        default:
            send_response(client_fd, "400 BAD REQUEST\n");
            break;
        }

        pthread_mutex_lock(&cds_dev.mutex);
        cds_dev.busy = 0;
        cds_dev.client_fd = -1;
        pthread_mutex_unlock(&cds_dev.mutex);
    }

    return NULL;
}