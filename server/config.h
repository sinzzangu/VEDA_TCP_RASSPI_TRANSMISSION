#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int port;
    int threshold;
} Config;

/* Read server.conf into cfg. Returns 0 on success, -1 on failure. */
int load_config(const char *path, Config *cfg);

#endif /* CONFIG_H */