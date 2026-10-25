#include <avr/io.h>
#include "os_input.h"
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

void manuell(void);
void SAWandler(void);
void TrackingWandler(void);

enum Type
{
    manual,
    tracking,
    saw
};

int main(void)
{
    os_initInput();
    enum Type select = saw;
    switch (select)
    {
    case manual:
        manuell();
        break;
    case tracking:
        while (true)
        {
            os_waitForInput();
            TrackingWandler();
        }
        break;
    case saw:
        while (true)
        {
            os_waitForInput();
            SAWandler();
        }
        break;
    }
}

void manuell(void)
{
    // init the registers
    DDRD = 0;     // DIP switch as input
    PORTD = 0xff; // Enable pullups
    DDRA = 0xff;  // LEDs as output
    DDRB = 0xff;  // R2R as output

    // forever
    while (true)
    {
        // set the LED to 0 when pressed -> copy inputs
        PORTA = PIND;
        // set the R2R to high if pressed -> inverse inputs;
        PORTB = ~PIND;
    }
}

void TrackingWandler(void)
{
    // prepares the comparator to be read
    DDRC &= ~1;
    PORTC |= 1;
    // init the LEDs and the comparing voltage
    DDRA = 0xff; // LEDs
    DDRB = 0xff; // R2R
    // give initial values (LEDs off, R2R to 0)
    PORTA = ~0;
    PORTB = 0;
    for (uint8_t i = 0; i < 255; i++)
    {
        if (PINC & 1) // if R2R voltage is higher than the input voltage we can stop
        {
            PORTA = ~i;
            return;
        }

        // write the LEDs
        PORTA = ~(i+1);
        // increase the R2R voltage
        PORTB = i+1;
        _delay_ms(50);
    }
}

void SAWandler(void)
{
    // prepares the comparator to be read
    DDRC &= ~1;
    PORTC |= 1;
    // init the LEDs and the comparing voltage
    DDRA = 0xff;
    DDRB = 0xff;

    // give initial values
    PORTA = ~0;
    PORTB = 0;

    uint8_t cur = 0b10000000;
    for (uint8_t i = 1; i <= 8; i++)
    {
        // set voltage to the halfpoint
        PORTB |= cur; // Only modify one bit
        PORTA &= ~cur;
		_delay_ms(50);

        if (PINC & 1) // if its below it we move onto the the lower half of the spectrum
        {
            PORTB &= ~cur;
            PORTA |= cur;
        }
        cur >>= 1;
    }
}
