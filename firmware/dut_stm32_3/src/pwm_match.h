#ifndef PWM_MATCH_H
#define PWM_MATCH_H
int  pwm_match_init(void);
void pwm_match_update(void);
void pwm_match_thread(void *a, void *b, void *c);
#endif