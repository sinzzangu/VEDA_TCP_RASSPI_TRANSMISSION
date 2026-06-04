#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "daemon.h"

int daemonize(const char *log_path) {
    /* 1st fork — parent exits, child continues in background */
    pid_t pid = fork();
    if (pid < 0) {
        perror("!![DAEMON] FIRST FORK ERROR");
        return -1;
    }
    if (pid > 0) {
        /* parent: print child PID and exit */
        printf("[DAEMON] server started in background (PID %d)\n", pid);
        printf("[DAEMON] logs: %s\n", log_path);
        exit(0);
    }

    /* child: become session leader */
    if (setsid() < 0) {
        perror("!![DAEMON] SETSID ERROR");
        return -1;
    }

    /* 2nd fork — prevent re-acquiring a controlling terminal */
    pid = fork();
    if (pid < 0) {
        perror("!![DAEMON] SECOND FORK ERROR");
        return -1;
    }
    if (pid > 0)
        exit(0); /* first child exits */

    /* file mode mask */
    umask(0);

    /* redirect stdin to /dev/null */
    int dev_null = open("/dev/null", O_RDONLY);
    if (dev_null >= 0) {
        dup2(dev_null, STDIN_FILENO);
        close(dev_null);
    }

    /* redirect stdout and stderr to the log file (append mode) */
    int log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) {
        /* can't write to log — fall back to /dev/null */
        log_fd = open("/dev/null", O_WRONLY);
        if (log_fd < 0)
            return -1;
    }
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);

    /* line-buffer stdout so log writes appear immediately */
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    return 0;
}