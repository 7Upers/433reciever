#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lib/uart.h"

#define LED PB5

volatile uint8_t packet[3] = { 0, 0, 0 };
volatile uint8_t byte = 0;
volatile uint8_t bit = 0;
volatile uint8_t newcmd = 0; //flag

void ir_worker(void)
{
	PORTB |= (1<<LED); //on

	printf("cmd = ");

	for (byte = 0; byte <= 2; byte++)
	{
		printf("%02x ", packet[byte]);
	}
	printf("\r\n");

	PORTB ^= (1<<LED); //toggle
}

// ** 433MHz RF Protocol **
// 25 bits = start (0) + 3 data bytes
// T = 2,45ms (4900tiks)
// 0 = H 661us + L 1768us (1322 + 3536 tiks)
// 1 = H 1874us + L 558us 
// F counter 2MHz


ISR(INT0_vect)
{
	cli();
	newcmd = 0;

	TCNT1 = 0;
	while ((PIND &(1<<PD2))&&(TCNT1 < 5000));
	if ((TCNT1 < 1100)||(TCNT1 > 1500)) return;
	//start 0 pulse

	TCNT1 = 0;
	while ((!(PIND &(1<<PD2)))&&(TCNT1 < 5000));
	if ((TCNT1 < 3000)||(TCNT1 > 4000)) return;
	//start 0 pause

	//clean cmd
	for (byte = 0; byte <= 2; byte++) packet[byte] = 0;

	for (byte = 0; byte <=2; byte++)
	{
		for (bit = 0; bit <=7; bit++)
		{
			TCNT1 = 0;
			while ((PIND &(1<<PD2))&&(TCNT1 < 5000));
			if (TCNT1 > 5000) return;
			if ( TCNT1 >= 2000 )
			{
				// logical 1
				packet[byte] |= (1<<bit);
			}
			//bit start pause

			TCNT1 = 0;
			while ((!(PIND &(1<<PD2)))&&(TCNT1 < 5000));
			if (TCNT1 > 5000) return;
			//bit end pulse
		}
	}

	ir_worker();
}

int main(void)
{
	uart_init(MYUBRR);

	//ir input
	DDRD &= ~(1<<PD2);
	PORTD &= ~(1<<PD2);

	//Set Up INT0
	//------------
	EICRA |= (1<<ISC00)|(1<<ISC01);	//The rising edge of INT0 generates an interrupt request.
	EIMSK|=(1<<INT0);		//Enable INT0

	//Setup Timer1
	//------------
	TCCR1A |= ((1<<COM1A1)|(1<<COM1A0)); //Mode : Normal
	TCCR1B |= (1<<CS11); //Prescaler : Fcpu/8
//	TIMSK1 |= (1<<TOIE1);    //Overflow Interrupt Enable


	//setup led output
	DDRB = (1<<LED);

	for (byte = 0; byte <= 4; byte++) packet[byte] = 0;

	sei();

	uint8_t n = 0;

	while (1)
	{
		//led turn on/off
		//PORTB ^= (1<<LED);

		_delay_ms(1000);
		printf("%d =============\r\n", n);
		n++;
	}
}
