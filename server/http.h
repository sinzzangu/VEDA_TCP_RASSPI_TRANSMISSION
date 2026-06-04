#ifndef HTTP_H
#define HTTP_H

/* Parse one HTTP request from the given buffer.
 *
 * Returns:
 *   1  — cmd_out contains a command ready to dispatch (POST /command)
 *   0  — request handled internally (GET /, favicon, OPTIONS)
 *  -1  — client disconnected cleanly (POST /disconnect)
 */
int http_parse(int fd, const char *request, char *cmd_out, int cmd_size);

#endif /* HTTP_H */