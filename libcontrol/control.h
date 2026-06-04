#ifndef CONTROL_H
#define CONTROL_H

/* 초기화 / 정리 */
int control_init(void);
void control_cleanup(void);

/* LED */
void led_init(void); /* internal — called only by control_init */
void led_on(void);
void led_off(void);
void led_brightness(int level); /* 0=LOW, 1=MID, 2=HIGH */

/* 부저 (Buzzer) */
void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_tone(int freq); /* play a specific frequency */

/* 조도센서 (디지털 임계값 모듈) */
void cds_init(void); /* internal — called only by control_init */
int read_cds(void);  /* HIGH = dark, LOW = bright */

/* 7세그먼트 (단일 자리) */
void seg_init(void); /* internal — called only by control_init */
void seg_display(int digit);
void seg_clear(void);

#endif /* CONTROL_H */