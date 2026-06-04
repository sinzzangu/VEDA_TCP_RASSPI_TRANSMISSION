#include <softPwm.h>
#include <wiringPi.h>

#include "control.h"

#define LED_PIN 1
#define PWM_RANGE 100

static const int LED_DUTY[3] = {10, 50, 100}; /* LOW, MID, HIGH */
static int led_level = 2;                     /* default HIGH   */

void led_init(void) {
    softPwmCreate(LED_PIN, 0, PWM_RANGE);
}

void led_on(void) {
    softPwmWrite(LED_PIN, LED_DUTY[led_level]);
}

void led_off(void) {
    softPwmWrite(LED_PIN, 0);
}

void led_brightness(int level) {
    if (level < 0 || level > 2)
        return;
    led_level = level;
    softPwmWrite(LED_PIN, LED_DUTY[level]);
}