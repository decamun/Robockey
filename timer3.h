#ifndef TIMER3
#define TIMER3

volatile uint16_t _max_value2;
void start_pwm3(uint16_t resolution, float duty_cycle);
void set_duty3(float duty_cycle);
void stop3();

#endif