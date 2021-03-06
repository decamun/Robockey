#include "m_general.h"
#include "ADC_Pele.h"
#include "m_bus.h"
#include "m_usb.h"

#define PT_THRESHOLD 93

static float puck_angle = 0;
static int see_puck = 0;

void update_puck_angle ()
{
  float PT_angles[7] = {-0.96, -0.48, 0, 3.14, 0, 0.48, 0.96};
  //{Wide Left, left spike, left front, back, right front, right spike, wide right}

  float PT_values[7] = {0,0,0,0,0,0,0};
  float PT_weighted[7] = {0,0,0,0,0,0,0};

  int total = 0;
  int i = 0;

  ADC0();
  PT_values[0] = ADC;
  ADC1();
  PT_values[1] = ADC;
  ADC4();
  PT_values[2] = ADC;
  ADC5();
  PT_values[3] = ADC;
  ADC6();
  PT_values[4] = ADC;
  ADC7();
  PT_values[5] = ADC;
  ADC8();
  PT_values[6] = ADC;


  m_usb_tx_string("\n");
  for(i = 2; i<6; i++){
    if(PT_values[i]<PT_THRESHOLD){
      PT_values[i] = 0;
    }
  }

  puck_angle = 0;
  total = 0;


  for(i = 0; i<7; i++){
    puck_angle += PT_values[i]*PT_angles[i];
    total += PT_values[i];
    m_usb_tx_int(PT_values[i]);
    m_usb_tx_string("\t");
  }
  puck_angle = puck_angle/total;

  m_usb_tx_int(total);
  m_usb_tx_string("\t");
  m_usb_tx_int((int)(puck_angle * 100));

  if(total > 5){
  	see_puck = 1;
  } else {
  	see_puck = 0;
  }
}

int get_see_puck() {
	return see_puck;
}

float get_puck_angle() {
	return puck_angle;
}

int puck_right(){
  return (int)(!check(PINB, 4));
}
int puck_middle(){
  return (int)(!check(PIND, 3));
}
int puck_left(){
  return (int)(!check(PINB, 5));
}
