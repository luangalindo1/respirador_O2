/*
 * SPRINT 2 Application.c
 *
 * Created: 05/03/2021 15:02:11
 * Author : LUAN FÁBIO MARINHO GALINDO
			118110382
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdlib.h>
#include <time.h>
#include <util/delay.h>

void controlLED (uint8_t *);
void showResp (uint8_t *);
void controlFreq (uint8_t **);

int main(void)
{
	//DDRD &= 0b00000; // PD0..4 = ENTRADA
	//PORTD |= 0b11111; // Habilita os resistores de pull-up das portas PD0..4
	//DDRC |= 0b1111111; // PC0..6 = SAÍDA
	//DDRB |= 0b11111111; // PB0..7 = SAÍDA
	srand(time(NULL)); // Garantindo a aleatoriedade a cada execução
	uint8_t FreqRespiracao = 5 + rand() % 25; // Definir um valor aleatório inicial
	
while        
 (1) 
    {
		showResp(&FreqRespiracao);
		controlLED(&FreqRespiracao);			
    }
}

void controlFreq(uint8_t **freq)
{
	DDRD &= 0b00000; // PD0..4 = ENTRADA
	PORTD |= 0b11111; // Habilita os resistores de pull-up das portas PD0..4
	
	if (!(PIND & (1<<0))) // Retorna 1 se PD0 == 1, e 0 cc
	{	
		if (**freq < 30)
			**freq = **freq + 1;
	}
	if (!(PIND & (1<<1))) // Retorna 1 se PD1 == 1, e 0 cc
	{	
		if (**freq > 5)
			**freq = **freq - 1;
	}
}
void controlLED (uint8_t *freq)
{
	DDRB |= 0b11111111; // PB0..7 = SAÍDA
	
	for (int i = 0; i <= 7; i++)
	{
		controlFreq(&freq);
		PORTB |= 1<<i;
		_delay_ms(60000/((*freq)*16));
	}
	for (int j = 7; j >= 0; j--)
	{
		controlFreq(&freq);
		PORTB &= ~(1<<j);
		_delay_ms(60000/((*freq)*16));
	}
}

void showResp(uint8_t *fresp) // mostra de 0 a 30 em hexa
{	
		DDRC |= 0b1111111; // PC0..6 = SAÍDA
		
		switch (*fresp)
		{
			case 0:
				PORTC = 0b00000000;
				break;
			case 1:
				PORTC = 0b00000001;
				break;
			case 2:
				PORTC = 0b00000010;
				break;
			case 3:
				PORTC = 0b00000011;
				break;
			case 4:
				PORTC = 0b00000100;
				break;
			case 5:
				PORTC = 0b00000101;
				break;
			case 6:
				PORTC = 0b00000110;
				break;
			case 7:
				PORTC = 0b00000111;
				break;
			case 8:
				PORTC = 0b00001000;
				break;
			case 9:
				PORTC = 0b00001001;
				break;
			case 10:
				PORTC = 0b00001010;
				break;
			case 11:
				PORTC = 0b00001011;
				break;
			case 12:
				PORTC = 0b00001100;
				break;
			case 13:
				PORTC = 0b00001101;
				break;
			case 14:
				PORTC = 0b00001110;
				break;
			case 15:
				PORTC = 0b00001111;
				break;
			case 16:
				PORTC = 0b00010000;
				break;
			case 17:
				PORTC = 0b00010001;
				break;
			case 18:
				PORTC = 0b00010010;
				break;
			case 19:
				PORTC = 0b00010011;
				break;	
			case 20:
				PORTC = 0b00010100;
				break;
			case 21:
				PORTC = 0b00010101;
				break;
			case 22:
				PORTC = 0b00010110;
				break;			
			case 23:
				PORTC = 0b00010111;
				break;
			case 24:
				PORTC = 0b00011000;
				break;
			case 25:
				PORTC = 0b00011001;
				break;
			case 26:
				PORTC = 0b00011010;
				break;
			case 27:
				PORTC = 0b00011011;
				break;
			case 28:
				PORTC = 0b00011100;
				break;
			case 29:
				PORTC = 0b00011101;
				break;									
			case 30:
				PORTC = 0b00011110;
				break;										
		}
}