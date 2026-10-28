#include "menu.h"

#include "adc.h"
#include "bin_clock.h"
#include "lcd.h"
#include "led.h"
#include "os_input.h"

#include <stdbool.h>
#include <util/delay.h>

#include <avr/io.h>
#include <stdint.h>

/*!
 *  Hello world program.
 *  Shows the string 'Hello World!' on the display.
 */
void helloWorld(void)
{
	// Repeat until ESC gets pressed
	while (true)
	{
		lcd_writeString("helloWorld");
		for (uint8_t i = 0; i < 10; i++)
		{
			if ((os_getInput() & 8) != 0)
			{
				os_waitForNoInput();
				return;
			}
			_delay_ms(50);

		}
		lcd_clear();
		for (uint8_t i = 0; i < 10; i++)
		{
			if ((os_getInput() & 8) != 0)
			{
				os_waitForNoInput();
				return;
			}
			_delay_ms(50);

		}
	}
	showMenu();
}

/*!
 *  Shows the clock on the display and a binary clock on the led bar.
 */
void displayClock(void) {
    uint16_t clockVal;
    while((os_getInput()&8) == 0){
		//generate clock bitstring
		clockVal = 0;
        clockVal |= getTimeSeconds();
        clockVal |= (getTimeMinutes()<<6);
        clockVal |= (getTimeHours()<<12);
        setLedBar(clockVal);
		
		//output to lcd
        lcd_clear();
        if(getTimeHours()<10){
            lcd_writeChar('0');
        }
        lcd_writeDec(getTimeHours());
        lcd_writeChar(':');
        if(getTimeMinutes()<10){
            lcd_writeChar('0');
        }
        lcd_writeDec(getTimeMinutes());
        lcd_writeChar(':');
        if(getTimeSeconds()<10){
            lcd_writeChar('0');
        }
        lcd_writeDec(getTimeSeconds());
        lcd_writeChar(':');
        if(getTimeMilliseconds()<100){
            lcd_writeChar('0');
        }
        if(getTimeMilliseconds()<10){
            lcd_writeChar('0');
        }
        lcd_writeDec(getTimeMilliseconds());
		
		
		_delay_ms(5);
    }
	showMenu();
}

/*!
 *  Shows the stored voltage values in the second line of the display.
 */
void displayVoltageBuffer(uint8_t displayIndex) {
	lcd_line2();
	if(displayIndex<99){
		lcd_writeChar('0');
	}
	if(displayIndex<9){
		lcd_writeChar('0');
	}
	lcd_writeDec(displayIndex+1);
	lcd_writeString("/100: ");
	lcd_writeVoltage(getStoredVoltage(displayIndex),1023,5);
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {
	int input = 0;
	int index = 0;
	int voltage;
	int ledbuffer;
    while((input & 8) == 0){
		//read voltage
		voltage = getAdcValue();
		//get input
		input = os_getInput();
		//write to screen
		lcd_clear();
		lcd_writeString("Voltage: ");
		lcd_writeVoltage(voltage, 1023, 5);
		//displayLEDs
		ledbuffer=0;
		while(voltage>68){
			ledbuffer <<=1;
			ledbuffer +=2;
			voltage -=68;
		}
		setLedBar(ledbuffer);
		//read inputs
		if((input &4)!=0 && index<99){
			index ++;
		}
		if((input &2)!=0 && index >0){
			index --;
		}
		if((input&1)!= 0){
			storeVoltage();
		}
		//display voltage buffer
		displayVoltageBuffer(index);
		//delay
		_delay_ms(100);
	}
	//return to menu
	setLedBar(0);
	showMenu();
}

/*! \brief Starts the passed program
 *
 * \param programIndex Index of the program to start.
 */
void start(uint8_t programIndex) {
    // Initialize and start the passed 'program'
    switch (programIndex) {
        case 0:
            lcd_clear();
            helloWorld();
            break;
        case 1:
            activateLedMask = 0xFFFF; // Use all LEDs
            initLedBar();
            initClock();
            displayClock();
            break;
        case 2:
            activateLedMask = 0xFFFE; // Don't use LED 0
            initLedBar();
            initAdc();
            displayAdc();
            break;
        default:
            break;
    }

    // Do not resume to the menu until all buttons are released
    os_waitForNoInput();
}

/*!
 *  Shows a user menu on the display which allows to start subprograms.
 */
void showMenu(void) {
    uint8_t pageIndex = 0;

    while (1) {
        lcd_clear();
        lcd_writeProgString(PSTR("Select:"));
        lcd_line2();

        switch (pageIndex) {
            case 0:
                lcd_writeProgString(PSTR("1: Hello world"));
                break;
            case 1:
                lcd_writeProgString(PSTR("2: Binary clock"));
                break;
            case 2:
                lcd_writeProgString(PSTR("3: Internal ADC"));
                break;
            default:
                lcd_writeProgString(PSTR("----------------"));
                break;
        }

        os_waitForInput();
        if (os_getInput() == 0b00000001) { // Enter
            os_waitForNoInput();
            start(pageIndex);
        } else if (os_getInput() == 0b00000100) { // Up
            os_waitForNoInput();
            pageIndex = (pageIndex + 1) % 3;
        } else if (os_getInput() == 0b00000010) { // Down
            os_waitForNoInput();
            if (pageIndex == 0) {
                pageIndex = 2;
            } else {
                pageIndex--;
            }
        }
    }
}
