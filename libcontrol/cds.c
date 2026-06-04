#include <wiringPiI2C.h>

#include "control.h"

#define I2C_ADDR 0x48

static int fd = -1;

void cds_init(void) {
    fd = wiringPiI2CSetup(I2C_ADDR);
}

int read_cds(void) {
    return wiringPiI2CRead(fd);
}