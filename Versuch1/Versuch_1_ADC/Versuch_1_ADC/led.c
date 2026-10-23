#include "led.h"

#include <avr/io.h>

uint16_t activateLedMask = 0xFFFF;

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {
    DDRA = (activateLedMask&0x00ff);
    DDRD = ((activateLedMask&0xff00)>>8);
    PORTA = (activateLedMask&0x00ff);
    PORTD = ((activateLedMask&0xff00)>>8);
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {
    //set masked bits to 0
    PORTA = ~(activateLedMask&0x00ff);
    PORTD = ~((activateLedMask&0xff00)>>8);
    //add the ones that are are missing
    PORTA |= ~((value & 0x00ff & activateLedMask));
    PORTD |= ~(((value & 0xff00& activateLedMask)>>8));
}
