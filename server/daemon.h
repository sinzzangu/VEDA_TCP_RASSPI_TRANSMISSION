#ifndef DAEMON_H
#define DAEMON_H

/* Detach the process from the controlling terminal.
 * Performs double-fork, setsid, and redirects stdout/stderr to log_path.
 * Working directory is NOT changed (so relative paths like "server.conf"
 * and "index.html" still work).
 * Returns 0 on success, -1 on failure. */
int daemonize(const char *log_path);

#endif /* DAEMON_H */