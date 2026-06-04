#include <wiringPi.h>

#include "control.h"

int control_init(void) {
    if (wiringPiSetup() != 0)
        return -1;

    led_init();
    cds_init();
    buzzer_init();
    seg_init();

    return 0;
}

void control_cleanup(void) {
    led_off();
    buzzer_off();
    seg_clear();
}
