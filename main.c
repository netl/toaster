#define F_CPU 1000000	//frequency for _delay_ms()

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>

#define true 1
#define false 0

int slot();	//tell if the slot is up
int button();	//read the state of the front button
int trimmer();	//read the trimmer and return it's value
void heater(uint8_t state);	//set the heater on/off
void hold(uint8_t state);	//hold the slot down
void debug(uint8_t state);	//set the state of the debug LED

volatile uint8_t data=0b00000000;
//             ||Heater clock (bits 5:0)
//             |hold slot (bit 6)
//             debug led (bit 7)
volatile uint16_t time=0;

int main(void)
{
	//set outputs
	DDRB=0xff;	//all out for now
	PORTB=0b000;	//all output low

	//make sure heater is OFF and the slot is not held down
	heater(0);

	//set inputs
	DDRA=0x00;	//all in
	PORTA=0b011;	//no pull-up for the trimmer
	
	//setup interupt for counting time
	TCCR1A=(1<<WGM12); //clear counter on compare match
	TCCR1B=(1<<CS11)|(1<CS10); //1MHZ/64=15.625kHz
	OCR1A=15625; //compare match at 1Hz
	TIMSK1=(1<<OCIE1A); //interupt on compare match a

	//setup adc for trimmer
	ADMUX=(1<<MUX1); //set PA2 for adc
	ADCSRB=(1<<ADLAR);	//set for reading of highest bits
	ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADATE);	//start free run mode

	//setup SPI slave
	DDRA|=(1<<5);
	USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USIOIE);
	USISR = (1<<USICNT0);
	sei();//enable interrupt

	//end of setup
	while(1) //be stuck forever.
	{
		if(!slot())     //hold the toast down
		{
			time=trimmer();	//set timer according to knob
			heater(1);
			debug(1);
			hold(1);
		      	while(button());        //wait for user input
			heater(0);
		      	hold(0);
			debug(0);
			while(!button());       //wait untill button is released
		}
	}
}

int slot()	//tell if the slot is up
{
	int status=PINA&0b1;
	return(status);
}

int button()	//read the state of the front button
{
	int state=(PINA>>1)&0b1;
	return(state);
}

int trimmer()	//read the trimmer and return it's value
{
	int pwr=ADCH;
	pwr=255-pwr;
	return(pwr);
}

void heater(uint8_t state)	//set the heater on/off
{				//WARNING: FIRE HAZARD!!!  
	if(state==true) 
		PORTB|=0b1;	//set heater pin high 
	else 
		PORTB&=~0b1;	//set heater pin low
}

void hold(uint8_t state)	//hold the slot down
{
	if(state==true)
		PORTB|=0b10;	//hold slot down
	else
		PORTB&=~0b10;	//do not hold slot down
}

void debug(uint8_t state)	//set the state of the debug LED
{
	if(state==1)
		PORTB|=0b100;	//on
	else if (state==0)
		PORTB&=~0b100;	//off
	else
		PORTB=(PORTB&~0b100)|(~PORTB&0b100);	//toggle
}

SIGNAL(TIM1_COMPA_vect)
{
	if(time > 0)
		time --;
	else
	{
		heater(0);
		debug(0);
	}
}

SIGNAL(USI_OVF_vect)
{
	data = USIBR;
	debug(3);
	USIDR=trimmer();
	USISR=(1<<USIOIF)|(1<<USICNT0);	//clear interrupt and set counter
}
