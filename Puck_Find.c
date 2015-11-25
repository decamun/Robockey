#include "m_general.h"
#include "ADC_Pele.h"
#include "m_bus.h"
#include "m_usb.h"

#define PT_THRESHOLD 93

void update_puck_angle ()
{
int ADC0_Val = 0;
int ADC1_Val = 0;
int ADC4_Val = 0;
int ADC5_Val = 0;
int ADC6_Val = 0;
int ADC7_Val = 0;
int ADC8_Val = 0;
int ADC9_Val = 0;

float PT_angles[7] = {-0.96, -0.48, 0, 3.14, 0, 0.48, 0.96};
//{Wide Left, left spike, left front, back, right front, right spike, wide right}

float PT_values[7] = {0,0,0,0,0,0,0};
float PT_weighted[7] = {0,0,0,0,0,0,0};
float puck_angle = 0;
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


        for(i = 2; i<6; i++){

            if(PT_values[i]<PT_THRESHOLD)
            {
                PT_values[i] = 0;
            }
        }

    puck_estimate = 0;
    total = 0;

        for(i = 0; i<7; i++){
            puck_angle += PT_values[i]*PT_angles[i];
            total += PT_values[i];
        }           
            puck_angle = angle_angle/total;
    }

}

int see_puck();


