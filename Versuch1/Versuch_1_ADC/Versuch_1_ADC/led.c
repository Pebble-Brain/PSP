#include "led.h"

#include <avr/io.h>

uint16_t activateLedMask = 0xFFFF;

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {
    DDRA = (uint8_t)activateLedMask;
    DDRD = (uint8_t)(activateLedMask>>8);
    PORTA = (uint8_t)activateLedMask;
    PORTD = (uint8_t)(activateLedMask>>8);
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {
    //set masked bits to 1
    PORTA |= (uint8_t)activateLedMask;
    PORTD |= (uint8_t)(activateLedMask>>8);
    //add the zeros where the leds are meant to be on
    PORTA &= ~(((uint8_t)activateLedMask)&((uint8_t)value));
    PORTD &= ~(((uint8_t)(activateLedMask>>8))&((uint8_t)(value>>8)));
}