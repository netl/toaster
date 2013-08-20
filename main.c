#define F_CPU 1000000	//frequency for _delay_ms()

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>

#define true 1
#define false 0

int slot();	//tell if the slot is up
int button();	//read the state of the front button
int trimmer();	//read the trimmer and return it's value
void heater(uint8_t state);	//set the heater on/off
void hold(uint8_t state);	//hold the slot down
void debug(uint8_t state);	//set the state of the debug LED

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

	//setup adc for trimmer
	ADMUX=(1<<MUX1); //set PA2 for adc
	ADCSRB=(1<<ADLAR);	//set for reading of highest bits
	ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADATE);	//start free run mode

	//setup i2c slave
	USICR=(1<<USIWM1)|(1<<USICS1)|(1<<USISIE)|(1<<USIOIE); //two wire and clock on rising edge
	USISR&=~0b1111;	//clear oveflow counter
	USISR|=8;	//8, so it will overflow after 8 clock cycles
	DDRA&=~0b1010000; //input on SDA & SCL
	PORTA&=~0b1010000;	//no pull-up since they're external

	sei();//enable interrupt

	//end of setup
	int time,a;

	while(1) //be stuck forever.
	{
		time=trimmer();
		if(!slot())
		{
			hold(1);
			debug(1);
			while(button());
			hold(0);
			debug(0);
		}
		debug(3);
		for(a=0;a<time;a++)
		{
			_delay_ms(1);
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
	pwr=256-pwr;
	return(pwr);
}

void heater(uint8_t state)	//set the heater on/off
{				//WARNING: FIRE HAZARD!!!  
	if(state==true) 
		PORTB|=0b1;	//set heater pin high 
	else 
		PORTB&=~0b10;	//set heater pin low
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

SIGNAL(USI_STR)
{
	//prep registers for i2c
}

SIGNAL(USI_OVF_vect)	//i2c clock counter overflow
{
	int data=USIBR;
}
