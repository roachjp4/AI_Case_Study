#include <avr/io.h>
#include <util/delay.h>

#define BUZZER_PIN DDB2  // Digital Pin 10 on Arduino Uno

void setup_pwm() {
	// TIMER 1 - 16 bit
	// OC1A and OC1B synced
	// EXAMPLE set PWM for 25% duty cycle @ 16bit
	// OCR1A = 0x3FFF;
	// set PWM for 75% duty cycle @ 16bit
	// OCR1B = 0xBFFF;
	
    	// set TOP to 16bit
	ICR1 = 0xFFFF; 

	OCR1A = 0x0000;
	OCR1B = 0x0000;

	// set none-inverting mode
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1); 
	// set Fast PWM mode using ICR1 as TOP - MODE 14
	TCCR1A |= (1 << WGM11);  
	TCCR1B |= (1 << WGM12) | (1 << WGM13); 
    
	// START the timer with no prescaler
	TCCR1B |= (1 << CS10);

 
	DDRB |= _BV(DDB2); /* set OC1B = Arduino_Pin10 pin as output - TIMER 1 */
}

void set_pwm_duty_cycle(uint16_t duty_cycle) {
    // Set PWM duty cycle with max volume 2047
    if (duty_cycle > 128*16) {
    	duty_cycle = 128*8;
    }
    OCR1B = duty_cycle;
}

void buzzer_on() {
    set_pwm_duty_cycle(128); // Set the volume (0 = off) 
}

void buzzer_off() {
    set_pwm_duty_cycle(0); // Turn off the PWM
}

int main(void) {
    setup_pwm();  // Initialize PWM

    while (1) {
        buzzer_on();              // Turn on the buzzer
        _delay_ms(500);           // Wait for 500 milliseconds
        buzzer_off();             // Turn off the buzzer
        _delay_ms(500);           // Wait for another 500 milliseconds
    }

    return 0;
}

