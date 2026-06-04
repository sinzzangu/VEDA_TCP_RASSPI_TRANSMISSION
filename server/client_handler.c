#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_handler.h"
#include "devices.h"
#include "http.h"
#include "protocol.h"

#define MAXDATASIZE 2048

void *client_thread(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    char buffer[MAXDATASIZE];
    char cmd_buf[64];
    int bytes_in;

    while (1) {
        // block until client sends data
        bytes_in = read(fd, buffer, MAXDATASIZE - 1);

        // case disconnected or error
        if (bytes_in <= 0) {
            printf("[SERVER] DISCONNECTING CLIENT fd: %d\n", fd);
            break;
        }

        buffer[bytes_in] = '\0';
        printf("[SERVER] DATA FROM CLIENT fd: %d\n", fd);
        printf("[SERVER] DATA: %s\n", buffer);

        memset(cmd_buf, 0, sizeof(cmd_buf));

        // detect HTTP request (browser connection)
        if (strncmp(buffer, "GET ", 4) == 0 ||
            strncmp(buffer, "POST ", 5) == 0 ||
            strncmp(buffer, "OPTIONS ", 8) == 0) {

            register_http_fd(fd);
            int r = http_parse(fd, buffer, cmd_buf, sizeof(cmd_buf));
            if (r == -1)
                break; // disconnect requested
            if (r == 0)
                continue; // handled internally
            // r == 1: cmd_buf has the command — fall through to dispatch

        } else {
            // CLI client: buffer is the command directly
            strncpy(cmd_buf, buffer, sizeof(cmd_buf) - 1);
            int l = strlen(cmd_buf);
            while (l > 0 && (cmd_buf[l - 1] == '\r' || cmd_buf[l - 1] == '\n'))
                cmd_buf[--l] = '\0';
        }

        // parse and dispatch command (same path for HTTP and CLI)
        Command cmd;
        if (parse_command(cmd_buf, &cmd) == -1) {
            printf("[SERVER] ERROR PARSING COMMAND: %s\n", cmd_buf);
            send_response(fd, "400 BAD REQUEST\n");
            continue;
        }

        printf("[SERVER] COMMAND: device=%d action=%d value=%d\n",
               cmd.device, cmd.action, cmd.value);

        int ret = devices_dispatch(fd, &cmd);
        if (ret == -1)
            send_response(fd, "503 BUSY\n");
        else if (ret == -2)
            send_response(fd, "400 BAD REQUEST\n");
    }

    // release any device locks held by this client
    unregister_http_fd(fd);
    devices_release(fd);
    close(fd);
    return NULL;
}