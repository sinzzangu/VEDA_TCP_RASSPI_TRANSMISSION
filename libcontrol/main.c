#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

void *link_library(void) {
    void *handle = dlopen("./libcontrol.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return NULL;
    }
    return handle;
}

void *get_library_function(void *handle, const char *name) {
    void *fn = dlsym(handle, name);
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "dlsym error: %s\n", error);
        exit(EXIT_FAILURE);
    }
    return fn;
}

int main(void) {
    void *handle = link_library();
    if (!handle)
        return 1;

    int (*control_init)(void) = get_library_function(handle, "control_init");
    void (*control_cleanup)(void) = get_library_function(handle, "control_cleanup");
    void (*led_on)(void) = get_library_function(handle, "led_on");
    void (*led_off)(void) = get_library_function(handle, "led_off");
    void (*led_brightness)(int) = get_library_function(handle, "led_brightness");
    int (*read_cds)(void) = get_library_function(handle, "read_cds");
    void (*buzzer_on)(void) = get_library_function(handle, "buzzer_on");
    void (*buzzer_off)(void) = get_library_function(handle, "buzzer_off");
    void (*seg_display)(int) = get_library_function(handle, "seg_display");
    void (*seg_clear)(void) = get_library_function(handle, "seg_clear");

    control_init();

    printf("CDS: %d\n", read_cds());
    sleep(5);

    printf("LOW\n");
    led_brightness(0);
    sleep(2);

    printf("MID\n");
    led_brightness(1);
    sleep(2);

    printf("HIGH\n");
    led_brightness(2);
    sleep(2);

    printf("OFF\n");
    led_off();
    // sleep(2);

    // printf("BUZZER ON\n");
    // buzzer_on();
    // sleep(2);

    // printf("BUZZER OFF\n");
    // buzzer_off();

    // printf("SEG 0-9\n");
    // for (int i = 0; i <= 9; i++) {
    //     seg_display(i);
    //     sleep(1);
    // }

    control_cleanup();

    digitalWrite(1, 10);
    sleep(10);

    dlclose(handle);
    return 0;
}