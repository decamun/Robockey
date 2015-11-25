//Puck Angle Estimation

/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include "m_general.h"
 #include "ADC_Pele.h"
 #include "m_bus.h"
 #include "m_usb.h"

int ADC0_Val = 0;
int ADC1_Val = 0;
int ADC4_Val = 0;
int ADC5_Val = 0;
int ADC6_Val = 0;
int ADC7_Val = 0;
int ADC8_Val = 0;
int ADC9_Val = 0;


int Puck_Angle(void)
{
        char rx_buffer;
        DDRE |= 1<<6;
        PORTE &= !(1<<6);
    /* insert your hardware initialization here */


        m_clockdivide(0);

        m_red(ON);
        ADC_init();
        m_red(OFF);


    m_green(ON);
    m_usb_init();                               // Initialize USB
    while(!m_usb_isconnected()){}               
    m_green(OFF);

clear(DDRB,4); // Setting B4 to input
set(PORTB,4); //Enabling Pull oup on B4

clear(DDRB,5); //Setting B5 to input
set(PORTB,5); //Enabling Pull oup on B5

clear(DDRD,3); //Setting D3 to input
set(PORTD,3); //Enabling Pull oup on D3

    for(;;){
        if(!check(PINB,4) || !check(PINB,5))
        {
            m_red(TOGGLE);
        }

        if(check(PIND,3))
        {
            m_green(TOGGLE);
        }

/*
        m_usb_tx_string("Switch 1");
        m_usb_tx_int(check(PINB,4));
        m_usb_tx_string(", Switch 1");
        m_usb_tx_int(check(PINB,4));
        m_usb_tx_string(", Distance Sensor");
        m_usb_tx_int(check(PIND,3));

        m_usb_tx_char('\t');

        m_green(TOGGLE);
        //m_wait(100);
        */


        while(!m_usb_rx_available());           // Wait for an indication from the computer
        rx_buffer = m_usb_rx_char();            // Read the packet from the computer
        ADC0();
        ADC0_Val = ADC; 
        ADC1();
        ADC1_Val = ADC; 
        ADC4();
        ADC4_Val = ADC; 
        ADC5();
        ADC5_Val = ADC; 
        ADC6();
        ADC6_Val = ADC; 
        ADC7();
        ADC7_Val = ADC; 
        ADC7();
        ADC7_Val = ADC;
        ADC8();
        ADC8_Val = ADC;
        //ADC9();
        //ADC9_Val = ADC;

        m_usb_rx_flush();                 // Flush the buffer

       
        if(rx_buffer == 1); {
        m_usb_tx_int(ADC0_Val);
        m_usb_tx_char('\t');
        m_usb_tx_int(ADC1_Val);
        m_usb_tx_char('\t');
        m_usb_tx_int(ADC4_Val);
        m_usb_tx_char('\t');
        m_usb_tx_int(ADC5_Val);
        m_usb_tx_char('\t');
        m_usb_tx_int(ADC6_Val);
        m_usb_tx_char('\t');
        m_usb_tx_int(ADC7_Val);
        m_usb_tx_char('\t');
        m_usb_tx_int(ADC8_Val);
        m_usb_tx_char('\n'); 


        }
/*

       m_usb_tx_string("\n ADC0 = ");
        m_usb_tx_int(ADC0_Val);
        m_usb_tx_string(", ADC1 = ");
        m_usb_tx_int(ADC1_Val);
        m_usb_tx_string(", ADC4 = ");
        m_usb_tx_int(ADC4_Val);
        m_usb_tx_string(", ADC5 = ");
        m_usb_tx_int(ADC5_Val);
        m_usb_tx_string(", ADC6 = ");
        m_usb_tx_int(ADC6_Val);
        m_usb_tx_string(", ADC7 = ");
        m_usb_tx_int(ADC7_Val);
        m_usb_tx_string(", ADC8 = ");
        m_usb_tx_int(ADC8_Val);


        /* insert your main loop code here */
    }
    return 0;   /* never reached */
}
