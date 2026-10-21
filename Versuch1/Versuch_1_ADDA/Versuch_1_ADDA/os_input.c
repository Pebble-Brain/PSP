#include "os_input.h"

#include <avr/io.h>
#include <stdint.h>

/*! \file
 * Everything that is necessary to get the input from the Buttons in a clean format.
 */


/*!
 *  A simple "Getter"-Function for the Buttons on the evaluation board.\n
 *
 *  \returns The state of the button in the lower bits of the return value.\n
 *  example: Button C1: -pushed:   00000001
 *                      -released: 00000000
 */
uint8_t os_getInput(void) {
    return (((~PINC)>>1)&1);
}

/*!
 *  Initializes DDR and PORT for input
 */
void os_initInput() {
    DDRC &= 0b11111101;
    PORTC |= 0b00000010;
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void os_waitForNoInput() {
    while(os_getInput()!=0){

    }
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void os_waitForInput() {
    while(os_getInput()==0){
        
    }
}
