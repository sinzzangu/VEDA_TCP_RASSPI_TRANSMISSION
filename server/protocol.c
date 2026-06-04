#include <stdlib.h>
#include <string.h>

#include "protocol.h"

int parse_command(const char *buf, Command *cmd) {
    char tmp[64];
    strncpy(tmp, buf, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    cmd->device = DEV_UNKNOWN;
    cmd->action = ACT_UNKNOWN;
    cmd->value = -1;

    char *device = strtok(tmp, " ");
    char *action = strtok(NULL, " ");
    char *value = strtok(NULL, " ");

    if (!device || !action)
        return -1;

    /* device */
    if (strcmp(device, "LED") == 0)
        cmd->device = DEV_LED;
    else if (strcmp(device, "BUZZER") == 0)
        cmd->device = DEV_BUZZER;
    else if (strcmp(device, "SEG") == 0)
        cmd->device = DEV_SEG;
    else if (strcmp(device, "CDS") == 0)
        cmd->device = DEV_CDS;
    else
        return -1;

    /* action */
    if (strcmp(action, "ON") == 0)
        cmd->action = ACT_ON;
    else if (strcmp(action, "OFF") == 0)
        cmd->action = ACT_OFF;
    else if (strcmp(action, "BRIGHT") == 0)
        cmd->action = ACT_BRIGHT;
    else if (strcmp(action, "READ") == 0)
        cmd->action = ACT_READ;
    else if (strcmp(action, "DISPLAY") == 0)
        cmd->action = ACT_DISPLAY;
    else if (strcmp(action, "CLEAR") == 0)
        cmd->action = ACT_CLEAR;
    else if (strcmp(action, "COUNTDOWN") == 0)
        cmd->action = ACT_COUNT;
    else if (strcmp(action, "AUTO") == 0)
        cmd->action = ACT_AUTO;
    else
        return -1;

    /* value — required for BRIGHT, DISPLAY, and COUNTDOWN */
    if (cmd->action == ACT_BRIGHT ||
        cmd->action == ACT_DISPLAY ||
        cmd->action == ACT_COUNT) {
        if (!value)
            return -1;
        cmd->value = atoi(value);
    }

    return 0;
}