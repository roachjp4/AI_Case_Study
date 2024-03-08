#define __DELAY_BACKWARD_COMPATIBLE__
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "lib/hd44780.h"
#include <avr/interrupt.h>

/* This is the Design By Contract macros.*/
#define DBC // Can turn off these macros by commenting out this line
#ifdef DBC
/* needs to be at main since we are going to use Pin13 as our LED to warn us on assert fails */
#define DBC_SETUP() \
	/* turn on Pin 13 as we will use to indicate assertion/error failed */ \
	DDRB |= _BV(DDD3); 

#define PRE_CONDITION_DBC(eval_expression, time_blink_delay) \
	while (!(eval_expression))  \
	{ \
		PORTB |= _BV(PORTB5); \
		my_delay_ms(time_blink_delay); \
		PORTB &= ~_BV(PORTB5); \
		my_delay_ms(time_blink_delay); \
	}

#define POST_CONDITION_DBC(eval_expression, time_blink_delay) \
	while (!(eval_expression))  \
	{ \
		PORTB |= _BV(PORTB5); \
		my_delay_ms(time_blink_delay); \
		PORTB &= ~_BV(PORTB5); \
		/* half the delay off on post condition */ \
		my_delay_ms(time_blink_delay/2); \
	}
#elif
/* These are empty for when turned off */
#define DBC_SETUP() {}
#define PRE_CONDITION(eval_expression, time_blink_delay) {}
#define POST_CONDITION(eval_expression, time_blink_delay) {}
#endif

#define SECOND_IN_ms 1000
#define MY_BUTTON1 PD5 
#define MY_BUTTON2 PD4 
//#define MY_BUTTON2 PD4 

void my_delay_ms( unsigned int delay);
short check_button_press_and_release(int button);
void display_line(int b2_pressed);
void menu();
void display_clock(); 
void init_timer();
void set_time();

unsigned long millis();

volatile unsigned long millis_counter = 0;
unsigned int hrs = 0;
unsigned int min = 0;
unsigned int sec = 0;

int main(void)
{
	//Setup
	LCD_Setup();
	set_time();	
	init_timer(); // Initialize the timer
	int b2_pressed = 0;
	LCD_Clear();
	
	while(1) 
	{
		while (1)
		{
			if (check_button_press_and_release(MY_BUTTON1))
			{
				LCD_Clear();
				break;
			}
			display_line(b2_pressed);
		}

		while (1)
		{
			if (check_button_press_and_release(MY_BUTTON1))
			{
				LCD_Clear();
				break;
			}
			display_clock();
		}
	}
}

void set_time()
{
	int b1_pressed = 0;
	while(b1_pressed < 2) {
		if (check_button_press_and_release(MY_BUTTON1)) 
		{
			b1_pressed++;
			LCD_Clear();
		}
		switch(b1_pressed) {
		
			case(0): 
			
			if (check_button_press_and_release(MY_BUTTON2)) {
				hrs++; 
			}
			
			LCD_GotoXY(0, 0);
			LCD_PrintInteger(hrs);
			LCD_PrintString(" HOUR");
			break;
			case(1): 
			
			if (check_button_press_and_release(MY_BUTTON2)) {
				min++; 
			}
			
			LCD_GotoXY(0, 0);
			LCD_PrintInteger(min);
			LCD_PrintString(" MIN");
			break;
		}
	}
}
void menu() {
	//Print two lines with class info
	uint8_t line;
	for (line = 0; line < 2; line++)
	{
		LCD_GotoXY(0, line);
		if (line == 0)
		{
			LCD_PrintString("BTN 1 CLR & DISP HELP");
			//LCD_PrintInteger(LCD_GetY());
		}
		else 
		{
			LCD_PrintString("BTN 2 TOGGLE DISP");
			//LCD_PrintInteger(LCD_GetY());
		}
	}
}

void display_line(int b2_pressed) 
{
	uint8_t menu_num = b2_pressed % 3;
	uint8_t line;
	switch(menu_num) 
	{
		case(0):
		LCD_GotoXY(0, 0);
		LCD_PrintString("ETHAN BARNES");
		break;
		case(1):
		//Print two lines with class info
		for (line = 0; line < 2; line++)
		{
			LCD_GotoXY(0, line);
			if (line == 0)
			{
				LCD_PrintString("BTN 2 PRESSED");
			}
			else 
			{
				LCD_PrintInteger(b2_pressed);
				LCD_PrintString(" TIMES");
				
			}
		}
		break;
		case(2):
		//Print two lines with class info
		for (line = 0; line < 2; line++)
		{
			LCD_GotoXY(0, line);
			if (line == 0)
			{
				LCD_PrintString("ECE 484");
			}
			else 
			{
				LCD_PrintString("SECTION A");
				
			}
		}
		break;
	}
}


ISR(TIMER1_COMPA_vect)
{
    millis_counter++;
}

void init_timer()
{
    // Configure Timer 1 for a 1ms interrupt
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC mode, prescaler = 64
    OCR1A = 250; // Timer compare value for 1ms at 16MHz
    TIMSK1 |= (1 << OCIE1A); // Enable Timer 1 compare match interrupt

    sei(); // Enable global interrupts
}

unsigned long millis()
{
    return millis_counter;
}

void display_clock() 
{
    // Retrieve current time information (you need to implement this part)
    // For example, you can use RTC (Real-Time Clock) library or other timekeeping mechanisms
    // and display the time on the LCD.
    // Calculate elapsed time in seconds
    unsigned long elapsedSeconds = millis() / 1000;
    // Placeholder for demonstration
    LCD_GotoXY(0, 0);
    LCD_PrintString("Current Time:");

    // Replace this with actual time information retrieval and formatting
    LCD_GotoXY(0, 1);

    LCD_PrintInteger(elapsedSeconds);
    LCD_PrintString(" seconds");
}

/* 
 * checks when a button on the D port is pressed assuming a pull-down in non-pressed state 
 * 
 * WIRING: input and resitor on same connection, Vcc on other connection
 */
short check_button_press_and_release(int button)
{
	int ret_val = 0;

	PRE_CONDITION_DBC(button >= 0, 6000);
	PRE_CONDITION_DBC(button < 8, 7000);

	if ((PIND & (1 << button)) != 0)
	{
		/* software debounce */
		_delay_ms(15);
		if ((PIND & (1 << button)) != 0)
		{
			/* wait for button to be released */
			while((PIND & (1 << button)) != 0)
				ret_val = 1;
		}
	}

	POST_CONDITION_DBC(ret_val == 1 || ret_val == 0, 5000);

	return ret_val;
}

/* 
 * Handles larger delays the _delay_ms can't do by itself (not sure how accurate)  
 * Note no DBC as this function is used in the DBC !!! 
 *
 * borrowed from : https://www.avrfreaks.net/forum/delayms-problem 
 * */
void my_delay_ms(unsigned int delay) 
{
	unsigned int i;

	for (i=0; i<(delay/10); i++) 
	{
		_delay_ms(10);
	}
	if (delay % 10) {
		_delay_ms(delay % 10);
	}
}
