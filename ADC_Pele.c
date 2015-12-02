//------------------------------------------Multiple ADC conversion stuff--------------------------------------------

//ADC intialization function

#include "m_general.h"

void ADC_init(void)
{
    clear(ADMUX, REFS1);    // Vref to Vcc
    set(ADMUX, REFS0);      // ^
    set(ADCSRA, ADPS2);     // Prescaler of 128 to give 125 kHz clock
    set(ADCSRA, ADPS1);     // ^
    set(ADCSRA, ADPS0);     // ^
    m_disableJTAG();        // Disabling F pins for ADC
}

void ADC0(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR0, ADC0D);                  // F0
    //set(PORTF,0);                       // Enable Pull up resistor
    clear(PORTF,0);                     // disable pull up resistor

    clear(ADCSRB, MUX5);                // Channel Selection - F0
    clear(ADMUX, MUX2);                 // ^
    clear(ADMUX, MUX1);                 // ^
    clear(ADMUX, MUX0);                 // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}

void ADC1(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR0, ADC1D);                  // F1
    //set(PORTF,1);
    clear(PORTF,1); 
    
    clear(ADCSRB, MUX5);                // Channel Selection - F1
    clear(ADMUX, MUX2);                 // ^
    clear(ADMUX, MUX1);                 // ^
    set(ADMUX, MUX0);                   // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}

void ADC4(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR0, ADC4D);                  // F4
    //   clear(PORTF,4);
    clear(PORTF,4);    


    clear(ADCSRB, MUX5);                // Channel Selection - F4
    set(ADMUX, MUX2);                   // ^
    clear(ADMUX, MUX1);                 // ^
    clear(ADMUX, MUX0);                 // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}

void ADC5(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR0, ADC5D);                  // F5
    clear(PORTF,5); 
    
    clear(ADCSRB, MUX5);                // Channel Selection - F5
    set(ADMUX, MUX2);                   // ^
    clear(ADMUX, MUX1);                 // ^
    set(ADMUX, MUX0);                   // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}

void ADC6(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR0, ADC6D);                  // F6
    clear(PORTF,6); 
    
    clear(ADCSRB, MUX5);                // Channel Selection - F6
    set(ADMUX, MUX2);                   // ^
    set(ADMUX, MUX1);                   // ^
    clear(ADMUX, MUX0);                 // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}

void ADC7(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR0, ADC7D);                  // F7
    clear(PORTF,7); 
    
    clear(ADCSRB, MUX5);                // Channel Selection - F7
    set(ADMUX, MUX2);                   // ^
    set(ADMUX, MUX1);                   // ^
    set(ADMUX, MUX0);                   // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}


void ADC8(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR2, ADC8D);                  // D4
     //   set(PORTD,4);
    clear(PORTD,4); 
    
    set(ADCSRB, MUX5);                  // Channel Selection - D4
    clear(ADMUX, MUX2);                 // ^
    clear(ADMUX, MUX1);                 // ^
    clear(ADMUX, MUX0);                 // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}

void ADC9(void)
{
    clear(ADCSRA, ADEN);
    
    set(DIDR2, ADC9D);                  // D6
       // set(PORTD,6);
    clear(PORTD,6); 
    
    set(ADCSRB, MUX5);                  // Channel Selection - D6
    clear(ADMUX, MUX2);                 // ^
    clear(ADMUX, MUX1);                 // ^
    set(ADMUX, MUX0);                   // ^
    
    set(ADCSRA, ADEN);                  // Enable ADC
    set(ADCSRA, ADSC);                  // Begin Conversion
    while(!check(ADCSRA,ADIF)){}        // wait for flag to be set
    set(ADCSRA,ADIF);                   // reset the flag
}
