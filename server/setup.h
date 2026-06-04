#ifndef SETUP_H
#define SETUP_H

#include "config.h"

/* Initialise the library, devices, and server socket.
 * Returns a bound and listening server_fd on success, -1 on failure. */
int server_setup(Config *cfg);

#endif /* SETUP_H */