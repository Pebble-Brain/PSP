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
void helloWorld(void) {
// Repeat until ESC gets pressed
	while(true){
		lcd_writeString("helloWorld");
		_delay_ms(500);
		lcd_clear();
		_delay_ms(500);
		if((os_getInput()&8) != 0){
			break;
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
        clockVal |= (getTimeHours()>>6);
        clockVal |= (getTimeHours()>>12);
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
    }
	showMenu();
}

/*!
 *  Shows the stored voltage values in the second line of the display.
 */
void displayVoltageBuffer(uint8_t displayIndex) {
	int index = displayIndex;
	lcd_line2();
	if(index<99){
		lcd_writeChar('0');
	}
	if(index<9){
		lcd_writeChar('0');
	}
	lcd_writeDec(index+1);
	lcd_writeString("/100 ");
	lcd_writeVoltage(getStoredVoltage(index), 1023, 5);
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {
    int adcResult;
    int ledValue;
    int index=0;
    int input;
    while((os_getInput()&8)==0){
        adcResult = getAdcValue();
        lcd_clear();
        lcd_writeString("Voltage: ");
        lcd_writeVoltage(adcResult, 1023, 5);
        displayVoltageBuffer(index);
        input = os_getInput();
        if((input & 1)!= 0){
            storeVoltage();
        }
        if((input&2)!= 0 && index <99){
            index ++;
        }
        if((input&4)!= 0 && index > 0){
            index --;
        }
        _delay_ms(100);
        ledValue = 0;
        while(adcResult>68){
            ledValue<<=1;
            ledValue+=2;
            adcResult-=68;
        }
        setLedBar(ledValue);
    }
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
