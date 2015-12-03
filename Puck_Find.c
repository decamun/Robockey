#include "m_general.h"
#include "ADC_Pele.h"
#include "m_bus.h"
#include "m_usb.h"
#include "Puck_Find.h"

#define PT_THRESHOLD 93

static float puck_angle = 0;
static int see_puck = 0;
static float puck_distance_cm;

void update_puck_angle ()
{
  float PT_angles[7] = {-0.96, -0.48, 0, 3.14, 0, 0.48, 0.96};
  //{Wide Left, left spike, left front, back, right front, right spike, wide right}

  float PT_values[7] = {0,0,0,0,0,0,0};

  int total = 0;
  int i = 0;

  ADC0();
  m_usb_tx_string("ADC0: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[6] = ADC;
  ADC1();
  m_usb_tx_string("ADC1: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[3] = ADC;
  ADC4();
  m_usb_tx_string("ADC4: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[2] = ADC;
  ADC5();
  m_usb_tx_string("ADC5: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[1] = ADC;
  ADC6();
  m_usb_tx_string("ADC6: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[5] = ADC;
  ADC7();
  m_usb_tx_string("ADC7: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[4] = ADC;
  ADC8();
  m_usb_tx_string("ADC8: ");
  m_usb_tx_int(ADC);
  m_usb_tx_string("\n\r");
  PT_values[0] = ADC;

  puck_angle = 0;
  total = 0;


  for(i = 0; i<7; i++){
    if(i != 4 && i != 2) {
      puck_angle += PT_values[i]*PT_angles[i];
      total += PT_values[i];
    }
    //m_usb_tx_int(PT_values[i]);
   // m_usb_tx_string("\t");
  }
  update_puck_distance(total + PT_values[2] + PT_values[4]);
  puck_angle = puck_angle/total;

  m_usb_tx_string("Distance: ");
  m_usb_tx_int((int)(get_puck_distance()));
  m_usb_tx_string("\n\r");

  m_usb_tx_string("Total: ");
  m_usb_tx_int(total + PT_values[2] + PT_values[4]);
  m_usb_tx_string("\n\r");

  if(total + PT_values[2] + PT_values[4] > 50){
  	see_puck = 1;
  } else {
  	see_puck = 0;
  }
}


//maps a distance to a total intensity value from Puck_Find
void update_puck_distance(int total_adc) {
  float x = (float)total_adc;
  puck_distance_cm = 0.000000000006f*x*x*x*x - 0.000000050911f*x*x*x + 0.000154567993f*x*x - 0.211193055181f*x + 131.517501154089f;

}

float get_puck_distance() {
  return puck_distance_cm;
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
