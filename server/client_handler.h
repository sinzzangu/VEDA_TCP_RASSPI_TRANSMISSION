#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

/* Thread function for handling one client connection (HTTP or CLI).
 * Argument is a malloc'd int* holding the client fd. */
void *client_thread(void *arg);

#endif /* CLIENT_HANDLER_H */