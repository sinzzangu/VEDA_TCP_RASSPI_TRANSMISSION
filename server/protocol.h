#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Devices */
#define DEV_LED 0
#define DEV_BUZZER 1
#define DEV_SEG 2
#define DEV_CDS 3
#define DEV_UNKNOWN -1

/* Actions */
#define ACT_ON 0
#define ACT_OFF 1
#define ACT_BRIGHT 2
#define ACT_READ 3
#define ACT_DISPLAY 4
#define ACT_CLEAR 5
#define ACT_COUNT 6
#define ACT_AUTO 7
#define ACT_UNKNOWN -1

typedef struct {
    int device; /* DEV_* */
    int action; /* ACT_* */
    int value;  /* optional: brightness level (0-2), seg digit (0-9) */
} Command;

/* Parse raw buffer into a Command. Returns 0 on success, -1 if unrecognised. */
int parse_command(const char *buf, Command *cmd);

#endif /* PROTOCOL_H */