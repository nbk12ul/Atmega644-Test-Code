#define F_CPU 8000000
#define xtalcpu 8000000

#define LINE2	0x41
#define LINE3	0x14
#define LINE4	0x54

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "hd44780.h"
#include "i2cmaster.h"
#include "uart.h"

//Global Variables
	char buffer[20];
	unsigned char MSB;
	unsigned char LSB;
	float temp_c;
	float temp_f;
	FILE lcd_stream = FDEV_SETUP_STREAM(lcd_putchar, 0, _FDEV_SETUP_WRITE);		//Setup LCD stream for writing
//	uart_init(38400);


//Function prototypes
static inline void initstuff(void);
int main(void);
void displaytemp(void);
void setuptempsensor(void);
void displaytime(void);



int main(void)
{

	initstuff();		//Call hardware init function
	uart_init(38400);
	uart_puts_p(PSTR("Testing Uart..."));
	lcd_clrscr();		//Clear LCD
//	setuptempsensor();
	//displaytemp();
	displaytime();
	

	for(;;)
	{
			uart_puts_p(PSTR("Testing Uart..."));
			uart_putc(0x3E);
			_delay_ms(1000);
	}		//Loop Forever
}

void displaytime(void)
{
	int time;
	char buffer[10];

	i2c_start_wait(0xD0);
	i2c_write(0x07);
	i2c_rep_start(0xD1);
	time = i2c_readNak();
	lcd_clrscr();
	itoa(time, buffer, 2);
	lcd_puts_P(PSTR("Read from clock:"));
	lcd_goto(LINE2);
	lcd_puts(buffer);
}


void setuptempsensor(void)
{
	char conf;
	char buffer[10];

	
	lcd_clrscr();
	lcd_puts_P(PSTR(" Temperature Sensor"));
	i2c_start_wait(0x90);
	i2c_write(0xAC);
	i2c_rep_start(0x91);
	conf = i2c_readNak();
	lcd_goto(LINE2);
	itoa(conf, buffer, 2);
	lcd_puts(buffer);

	lcd_goto(LINE4);
	lcd_putc(0x3E);
}

void displaytemp(void)
{
	i2c_start_wait(0x90);				//Start communications
	i2c_write(0x51);					//Start Temp Sample
	_delay_ms(75);						//Wait for sample
	i2c_rep_start(0x90);				//Communicate restart
	i2c_write(0xAA);					//Put new sample in registers
	i2c_rep_start(0x91);				//Communicate read mode
	MSB = i2c_readAck();				//Load MSB Byte
	LSB = i2c_readNak();				//Load LSB Byte
	if(MSB >= 0x80) 					//If MSB is 1 result is negative
	{
		temp_c = (float)(((MSB << 8) + LSB) - 65536) / 256;		//Make result Negative
	}
	else
	{
		temp_c = (float)((MSB << 8 )+ LSB) / 256;		//Shift all bytes by 8
	}
	temp_f = (float)(temp_c * 9/5) + 32;			//Convert for f
	fprintf_P(&lcd_stream, PSTR("Temp C = %.3f"), temp_c);		//Print C
	lcd_goto(LINE2);											//Move to line 2
	fprintf_P(&lcd_stream, PSTR("Temp F = %.2f"), temp_f);		//Print F
}

static inline void initstuff(void)
{
	DDRD = 0xFF;		//Set PortD as Output
	DDRB = 0xFF;		//Set PortB as Output
	PORTB = 0x01;		//Turn PB0 hight to Light "Power LED"
	lcd_init();			//Initialise the LCD
	i2c_init();
}
