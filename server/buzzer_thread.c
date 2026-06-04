#include <stdio.h>
#include <unistd.h>

#include "devices.h"

/* ------------------------------------------------------------------ */
/*  Happy Birthday melody                                              */
/* ------------------------------------------------------------------ */
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523

static const int melody[] = {
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4,
    NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4,
    NOTE_B4, NOTE_B4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4};

static const int durations[] = {
    8, 8, 4, 4, 4, 2,
    8, 8, 4, 4, 4, 2,
    8, 8, 4, 4, 4, 4, 4,
    8, 8, 4, 4, 4, 2};

#define BEAT_SPEED 1200   /* ms for a whole note — increase to slow down */
#define NOTE_GAP_US 50000 /* 50ms pause between notes                    */
#define TOTAL_NOTES (int)(sizeof(melody) / sizeof(melody[0]))

/* ------------------------------------------------------------------ */
/*  buzzer_thread_fn                                                   */
/* ------------------------------------------------------------------ */
void *buzzer_thread_fn(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&buzzer_dev.mutex);
        while (!buzzer_dev.busy)
            pthread_cond_wait(&buzzer_dev.cond, &buzzer_dev.mutex);

        Command cmd = buzzer_dev.cmd;
        int client_fd = buzzer_dev.client_fd;
        pthread_mutex_unlock(&buzzer_dev.mutex);

        switch (cmd.action) {

        case ACT_ON: {
            send_response(client_fd, "200 OK\n"); /* respond immediately */

            for (int i = 0; i < TOTAL_NOTES; i++) {
                pthread_mutex_lock(&buzzer_dev.mutex);
                int should_cancel = buzzer_dev.cancel;
                pthread_mutex_unlock(&buzzer_dev.mutex);
                if (should_cancel)
                    break;

                int note_duration = (BEAT_SPEED / durations[i]) * 1000;
                fns.buzzer_tone(melody[i]);
                usleep(note_duration);
                fns.buzzer_off();
                usleep(NOTE_GAP_US);
            }
            fns.buzzer_off();
            break; /* no second response */
        }

        case ACT_OFF:
            fns.buzzer_off();
            send_response(client_fd, "200 OK\n");
            break;

        default:
            send_response(client_fd, "400 BAD REQUEST\n");
            break;
        }

        pthread_mutex_lock(&buzzer_dev.mutex);
        buzzer_dev.busy = 0;
        buzzer_dev.client_fd = -1;
        pthread_mutex_unlock(&buzzer_dev.mutex);
    }

    return NULL;
}