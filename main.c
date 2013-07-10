#include <avr/io.h>

int main(void)
{
	//set outputs
	DDRB=0xff;	//all out for now
	PORTB=0b000;	//all output low
	//make sure heater is OFF
	heater(0);

	//set inputs
	DDRA=0x00;	//all in
	PORTA=0b011;	//no pull-up for the trimmer

	//setup adc for trimmer
	ADMUX=(1<<MUX1); //set PA2 for adc
	ADCSRB=(1<<ADLAR);
	ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADATE);
	//setup i2c

	while(1); //be stuck forever.
}

int trimmer()	//read the trimmer and return it's value
{
	int pwr=ADCH;
	return(pwr);
}

int slot()	//tell if the slot is up
{
	int status=PORTA&0b1;
	return(status);
}

void heater(uint8_t state)	//set the heater on/off
{				//WARNING: FIRE HAZARD!!!  
	if(state==TRUE) 
		PORTA|=0b1;	//set heater pin high 
	else 
		PORTA&=0xfe;	//set heater pin low
}
