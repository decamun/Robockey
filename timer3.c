#include "m_general.h"
#include "timer3.h"


volatile uint16_t _max_value3 = 0;
static int already_on_3 = 0;

void start_pwm3(uint16_t resolution, float duty_cycle) {
  _max_value3 = resolution; //save resolution

  ICR3 = resolution;  //frequency
  OCR3A = (uint16_t)(((float)resolution) * duty_cycle); //duty cycle

  if(!already_on_3) {
    //timer mode: 14 -> 1110
    set(TCCR3B, WGM33); //1
    set(TCCR3B, WGM32); //1
    set(TCCR3A, WGM31); //1
    clear(TCCR3A, WGM30); //0

    //output options C6: 10
    set(TCCR3A, COM3A1);   //1
    clear(TCCR3A, COM3A0); //0

    //clock prescaler: /8 -> 010
    clear(TCCR3B, CS32);    //0
    set(TCCR3B, CS31);  //1
    clear(TCCR3B, CS30);  //0
    set(DDRC, 6); //set port C6 output
    already_on_3 = 1;
  }
}

void set_duty3(float duty_cycle) {
  OCR3A = (uint16_t)(((float)_max_value3) * duty_cycle); //duty OCR3A
}

//stop timer3
void stop3() {
  //clock prescaler: 0 -> 000
  clear(TCCR3B, CS32);   //0
  clear(TCCR3B, CS31);   //0
  clear(TCCR3B, CS30);   //0
  clear(DDRC, 6);
  already_on_3 = 0;
}
