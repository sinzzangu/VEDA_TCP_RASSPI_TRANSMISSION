/* CREATED AT 06/22 VERSION 4
 * server.c — startup only.
 *   1. daemonize → run in background, log to server.log
 *   2. server_setup → load library, init devices, open socket
 *   3. accept loop → spawn detached client_thread per connection
 * All client and HTTP handling lives in client_handler.c.
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client_handler.h"
#include "config.h"
#include "daemon.h"
#include "setup.h"

int main(void) {
    // load config (before daemonize so error messages still go to terminal)
    Config cfg;
    if (load_config("server.conf", &cfg) == -1)
        return 1;

    // detach from terminal, redirect stdout/stderr to server.log
    if (daemonize("server.log") == -1)
        return 1;

    // initialise library, devices, and socket
    int server_fd = server_setup(&cfg);
    if (server_fd == -1)
        return 1;

    struct sockaddr_in client_addr;
    socklen_t addr_len;
    pthread_t tid;

    printf("\t [SERVER] WAITING FOR CONNECTIONS...\n");

    while (1) {
        addr_len = sizeof(client_addr);

        int *client_fd = malloc(sizeof(int));
        if (!client_fd) {
            perror("!![SERVER] MALLOC ERROR");
            continue;
        }

        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_fd == -1) {
            perror("!![SERVER] ACCEPT ERROR");
            free(client_fd);
            continue;
        }

        printf("[SERVER] NEW CONNECTION from IP: %s\n",
               inet_ntoa(client_addr.sin_addr));

        if (pthread_create(&tid, NULL, client_thread, client_fd) != 0) {
            perror("!![SERVER] PTHREAD_CREATE ERROR");
            free(client_fd);
            continue;
        }
        pthread_detach(tid);
    }

    return 0;
}