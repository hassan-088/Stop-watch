/*
 * main.c
 *
 *      Created on: Sep 14, 2024
 *      Author: mohamed hassan
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;
unsigned char count_down =0;
unsigned int last_TC1 = 0;

typedef unsigned char unit8;


// Initialize Timer1 in CTC mode
void timer1_init_CTC_mode() {

	/*set FOC1A for CTC mode No PWM*/
	TCCR1A = (1 << FOC1A);
		/* WGM12 = 1 => using mode 4 in timer1 (CTC) and compare match in OCR1A  */
		/* (CS10 = 1 && CS12 = 1) => pre-scaler 1024 */
	TCCR1B |= (1 << CS12) | (1 << CS10) | (1 << WGM12);
		/* start timer from 0 */
		TCNT1 = 0;
		/* set compare match value for 1 second */
		OCR1A = 15625;
		/* enable output compare A match interrupt */
		TIMSK |= (1 << OCIE1A);
		/* enable global interrupt */
		SREG |= (1 << 7);
}
// Timer1 Compare Match A ISR to update the time
ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0;
	OCR1A = 15625;
	if (count_down) {
	            if (seconds == 0 && hours == 0 && minutes == 0) {
	                PORTD |= (1 << PD0);
	            }

	            if (seconds > 0) {
	                seconds--;
	            } else if (minutes > 0) {
	                seconds = 59;
	                minutes--;
	            } else if (hours > 0) {
	                seconds = 59;
	                minutes = 59;
	                hours--;
	            }
	        }
	        // Count Up Mode
	        else {
	            seconds++;
	            if (seconds == 60) {
	                seconds = 0;
	                minutes++;
	            }
	            if (minutes == 60) {
	                minutes = 0;
	                hours++;
	            }
	            if (hours == 24) {
	                hours = 0;
	            }
	        }
	}
void INT0_INIT(){ //RESET

	DDRD &= ~(1<<PD2);
	PORTD |= (1 << PD2);
	GICR  |= (1<<INT0);
	MCUCR |= (1<<ISC01);
	MCUCR &= ~(1 << ISC00);

}
ISR(INT0_vect) {
	seconds = 0;
    minutes = 0;
    hours = 0;
    TCNT1 = 0;

}
void INT1_INIT(){  // paused

	DDRD &= ~(1 << PD3);
	GICR  |= (1<<INT1);
	MCUCR |= (1<<ISC11) | (1<<ISC10);

}
ISR(INT1_vect) {

	TCCR1B = 0;  // Stop Timer1
	last_TC1 = TCNT1;  // Store current Timer1 counter value

}
void INT2_INIT(){   // resumed

	DDRB &= ~(1 << PB2);
	GICR  |= (1<<INT2);
	MCUCSR &= ~(1<<ISC2);

}
ISR(INT2_vect) {

	 TCNT1 = last_TC1;  // Restore Timer1 counter value
	 TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);  // Restart Timer1

}
// Display seconds on 7-segment displays
void display_seconds() {
    PORTA &= ~(0x3F);  // Disable all displays
    int first_digit = seconds % 10;
    int second_digit = seconds / 10;

    PORTA = (PORTA & 0xC0) | (1 << PA4);  // Enable fifth display
    PORTC = (PORTC & 0xF0) | (second_digit & 0x0F);  // Send tens digit to BCD

    _delay_ms(2);

    PORTA = (PORTA & 0xC0) | (1 << PA5);  // Enable sixth display
    PORTC = (PORTC & 0xF0) | (first_digit & 0x0F);  // Send ones digit to BCD

    _delay_ms(2);  // Minimal delay for refresh
}
void display_minutes() {
    PORTA &= ~(0x3F);  // Disable all displays
    int first_digit = minutes % 10;
    int second_digit = minutes / 10;

    PORTA = (PORTA & 0xC0) | (1 << PA3);  // Enable fourth display
    PORTC = (PORTC & 0xF0) | (first_digit & 0x0F);

    _delay_ms(2);

    PORTA = (PORTA & 0xC0) | (1 << PA2);  // Enable third display
    PORTC = (PORTC & 0xF0) | (second_digit & 0x0F);

    _delay_ms(2);
}
void display_hours() {
    PORTA &= ~(0x3F);  // Disable all displays
    int first_digit = hours % 10;
    int second_digit = hours / 10;

    PORTA = (PORTA & 0xC0) | (1 << PA1);  // Enable second display
    PORTC = (PORTC & 0xF0) | (first_digit & 0x0F);

    _delay_ms(2);

    PORTA = (PORTA & 0xC0) | (1 << PA0);  // Enable first display
    PORTC = (PORTC & 0xF0) | (second_digit & 0x0F);

    _delay_ms(2);
}

int main(){
	DDRC |= 0X0F;
	DDRA |= 0X3F;

	DDRB |= 0X00;
	PORTB = 0XFF;

	DDRD |= (1 << PD0);

	DDRD |= (1 << PD4) | (1 << PD5);

	PORTA &= ~(0x3F);
	PORTC &= 0xF0;

	timer1_init_CTC_mode();
	INT0_INIT();
	INT1_INIT();
	INT2_INIT();

	 unit8 button_pressed_PB7 = 0;
	 unit8 button_pressed_PB1 = 0;  // Flag for Hours increment (PB1)
	 unit8 button_pressed_PB0 = 0;  // Flag for Hours decrement (PB0)
	 unit8 button_pressed_PB4 = 0;  // Flag for Minutes increment (PB4)
	 unit8 button_pressed_PB3 = 0;  // Flag for Minutes decrement (PB3)
	 unit8 button_pressed_PB6 = 0;  // Flag for Seconds increment (PB6)
	 unit8 button_pressed_PB5 = 0;  // Flag for Seconds decrement (PB5)


while(1){


       // Display time
       display_seconds();
       display_minutes();
       display_hours();

       if(seconds != 0 || hours != 0 || minutes != 0)
               	PORTD &= ~(1 << PD0);

       // Handle mode toggle with debouncing
              if (!(PINB & (1 << PB7)) && !button_pressed_PB7) {
                  _delay_ms(20);  // Debounce delay
                  if (!(PINB & (1 << PB7))) {  // Check if still pressed after debounce
                      count_down = !count_down;  // Toggle count mode
                      button_pressed_PB7 = 1;  // Set the flag to indicate button is pressed
                  }
              }
              if (PINB & (1 << PB7)) {
                  button_pressed_PB7 = 0;
              }

              // LEDs for Countup and CountDown
              if (count_down) {
                  PORTD |= (1 << PD5);  // Light up the 'count down' LED
                  PORTD &= ~(1 << PD4);  // Turn off the 'count up' LED
              } else {
                  PORTD |= (1 << PD4);  // Light up the 'count up' LED
                  PORTD &= ~(1 << PD5);  // Turn off the 'count down' LED
              }



              // Decrement hours with PB0
              if (!(PINB & (1 << PB0)) && !button_pressed_PB0) {
                  _delay_ms(20);  // Debounce delay
                  if (!(PINB & (1 << PB0))) {
                      if (hours > 0) hours--;
                      button_pressed_PB0 = 1;  // Set the flag to indicate button is pressed
                  }
              }
              if (PINB & (1 << PB0)) {
                  button_pressed_PB0 = 0;  // Reset flag once the button is released
              }



              // Increment hours with PB1
              if (!(PINB & (1 << PB1)) && !button_pressed_PB1) {
                  _delay_ms(20);  // Debounce delay
                  if (!(PINB & (1 << PB1))) {
                      hours++;
                      button_pressed_PB1 = 1;  // Set the flag to indicate button is pressed
                  }
              }
              if (PINB & (1 << PB1)) {
                  button_pressed_PB1 = 0;  // Reset flag once the button is released
              }




              // Increment Minutes With PB4
              if(!(PINB & (1 << PB4)) && !button_pressed_PB4){
                  _delay_ms(20);  // Debounce delay
                  if(!(PINB & (1 << PB4))){
                      if(minutes == 59){
                          minutes = 0;
                          hours++;
                      }
                      else {
                          minutes++;
                      }
                      button_pressed_PB4 = 1;  // Set the flag to indicate button is pressed
                  }
              }
              if(PINB & (1 << PB4)){
                  button_pressed_PB4 = 0;  // Reset flag once the button is released
              }



              // Decrement Minutes With PB3
              if(!(PINB & (1 << PB3)) && !button_pressed_PB3){
                  _delay_ms(20);  // Debounce delay
                  if(!(PINB & (1 << PB3))){
                      if(minutes > 0){
                          minutes --;
                      }

                      button_pressed_PB3 = 1;  // Set the flag to indicate button is pressed
                  }
              }


              if(PINB & (1 << PB3)){
                  button_pressed_PB3 = 0;  // Reset flag once the button is released
              }






              // Increment Seconds With PB6


              if(!(PINB & (1 << PB6)) && !button_pressed_PB6){
                  _delay_ms(20);  // Debounce delay
                  if(!(PINB & (1 << PB6))){
                      if(seconds == 59){
                          seconds = 0;
                          minutes++;

                      }
                      else
                          seconds++;

                      button_pressed_PB6 = 1;  // Set the flag to indicate button is pressed
                  }
              }


              if(PINB & (1 << PB6)){
                  button_pressed_PB6 = 0;  // Reset flag once the button is released
              }




              // Decrement Second with PB5

              if(!(PINB & (1 << PB5)) && !button_pressed_PB5){
                  _delay_ms(20);  // Debounce delay
                  if(!(PINB & (1 << PB5))){

                      if(seconds > 0)
                          seconds--;
                      button_pressed_PB5 = 1;  // Set the flag to indicate button is pressed
                  }
              }


              if(PINB & (1 << PB5)){
                  button_pressed_PB5 = 0;  // Reset flag once the button is released
              }
          }

      }


