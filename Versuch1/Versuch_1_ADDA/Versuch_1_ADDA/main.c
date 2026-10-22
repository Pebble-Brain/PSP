#include <avr/io.h>
#include "os_input.h"
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

void manuell(void);
void SAWandler(void);
void TrackingWandler(void);


int main(void) {
    os_initInput();
    int select = 1;
    switch(select){
        case 0:
            manuell();
            break;
        case 1:
            while(true){
                os_waitForInput();
                TrackingWandler();
            }
            break;
        default:
            while(true){
                os_waitForInput();
                SAWandler();
            }
            break;
    }
}

void manuell(void){
    //init the registers
    DDRD = 0;
    PORTD = 0xff;
    DDRA = 0xff;
    DDRB = 0xff;

    //forever
    while(true){
        //set the LED to 0 when pressed -> copy inputs
        PORTA = PIND;
        //set the R2R to hight if pressed -> inverse inputs;
        PORTB = ~PIND;
    }
}


void TrackingWandler(void){
    //prepares the comparator to be read
    DDRC &= ~1;
    PORTC |= 1;
    //init the LEDs and the comparing voltage
    DDRA = 0xff;
    DDRB = 0xff;
    //give initial values
    PORTA = ~0;
    PORTB = 0;
    for(int i =0; i<256; i++){
        if((PINC&1) != 0){
            //if the direction of tracking changes i.E the measure voltage has been exceeded
            return;
        }
        //this only happens if the new voltage was not larger than the measure voltage

        //write the LEDs
        PORTA = ~((uint8_t)i);
        //increase the compare voltge
        PORTB = (uint8_t)i;
        _delay_ms(50);
    }
}
void SAWandler(void){
    //prepares the comparator to be read
    DDRC &= ~1;
    PORTC |= 1;
    //init the LEDs and the comparing voltage
    DDRA = 0xff;
    DDRB = 0xff;

    //give initial values
    PORTA = ~0;
    PORTB = 0;
    
    uint8_t cur = 0b10000000;
    for(int i = 1; i<=8; i++){
        //set voltage to the halfpoint
        PORTB |= cur;
        PORTA &= ~cur;
        if((PINC&1) !=0){
            //if its below it we move onto the the lower half of the spectrum
            PORTB &= ~cur;
            PORTA |= cur;
        }
        cur >>=1;
    }
}
