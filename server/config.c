#include <stdio.h>
#include <string.h>

#include "config.h"

int load_config(const char *path, Config *cfg) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("load_config: fopen");
        return -1;
    }

    char key[32];
    int val;

    while (fscanf(f, "%31s %d", key, &val) == 2) {
        if (strcmp(key, "port") == 0)
            cfg->port = val;
        else if (strcmp(key, "threshold") == 0)
            cfg->threshold = val;
    }

    fclose(f);
    return 0;
}