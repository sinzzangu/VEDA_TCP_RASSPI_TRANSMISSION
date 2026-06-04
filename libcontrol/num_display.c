#include <wiringPi.h>

#include "control.h"

/* WiringPi pin for each segment: a, b, c, d, e, f, g */
static const int SEG_PINS[7] = {27, 28, 29, 25, 24, 23, 22};

/* Segment patterns for 0-9, common ANODE: 0 = LOW = on, 1 = HIGH = off */
static const int SEG_DIGITS[10][7] = {
    {0, 0, 0, 0, 0, 0, 1}, /* 0 */
    {1, 0, 0, 1, 1, 1, 1}, /* 1 */
    {0, 0, 1, 0, 0, 1, 0}, /* 2 */
    {0, 0, 0, 0, 1, 1, 0}, /* 3 */
    {1, 0, 0, 1, 1, 0, 0}, /* 4 */
    {0, 1, 0, 0, 1, 0, 0}, /* 5 */
    {0, 1, 0, 0, 0, 0, 0}, /* 6 */
    {0, 0, 0, 1, 1, 1, 1}, /* 7 */
    {0, 0, 0, 0, 0, 0, 0}, /* 8 */
    {0, 0, 0, 0, 1, 0, 0}, /* 9 */
};

void seg_init(void) {
    for (int i = 0; i < 7; i++)
        pinMode(SEG_PINS[i], OUTPUT);
    seg_clear();
}

void seg_display(int digit) {
    if (digit < 0 || digit > 9)
        return;
    for (int i = 0; i < 7; i++)
        digitalWrite(SEG_PINS[i], SEG_DIGITS[digit][i]);
}

void seg_clear(void) {
    for (int i = 0; i < 7; i++)
        digitalWrite(SEG_PINS[i], HIGH);
}