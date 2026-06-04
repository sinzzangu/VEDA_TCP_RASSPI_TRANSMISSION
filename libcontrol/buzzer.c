#include <softTone.h>

#include "control.h"

#define BUZZER_PIN 6
#define BUZZER_FREQ 440

void buzzer_init(void) {
    softToneCreate(BUZZER_PIN);
}

void buzzer_on(void) {
    softToneWrite(BUZZER_PIN, BUZZER_FREQ);
}

void buzzer_off(void) {
    softToneWrite(BUZZER_PIN, 0);
}

void buzzer_tone(int freq) {
    softToneWrite(BUZZER_PIN, freq);
}