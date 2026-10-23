#include "led.h"

#include <avr/io.h>

uint16_t activateLedMask = 0xFFFF;

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {
    DDRA = 0xff;
    DDRD = 0xff;
    PORTA = 0xff;
    PORTD = 0xff;
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {
    PORTA = ~(value & 0x00ff);
    PORTD = ~((value & 0xff00)>>8);
}
