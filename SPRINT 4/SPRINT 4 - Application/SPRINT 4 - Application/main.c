/*
 * SPRINT 4 - Application.c
 *
 * Created: 24/03/2021 11:31:56
 * Author : LUAN F�BIO MARINHO GALINDO
 *			118110382
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nokia5110.h"

void controlLED2 (uint8_t *);
void showResp3 (uint8_t);
void mydelay (uint32_t);

uint8_t FreqRespiracao = 5; // Definir um valor inicial
uint8_t bpm = 0;
uint32_t tempo_ms = 0;

ISR(INT0_vect) // interrup��o externa 0, quando o bot�o � pressionado, a freq aumenta
{
	if (FreqRespiracao < 30)
	{
		while(!(PIND & (1<<2))) // Garantir que a soma continue, caso o bot�o se mantenha pressionado
		{
			_delay_ms(150);
			if (FreqRespiracao < 30)
				FreqRespiracao++;
			showResp3(FreqRespiracao);
		}
	}
}
ISR(INT1_vect) // interrup��o externa 1, quando o bot�o � pressionado, a freq diminui
{
	if (FreqRespiracao > 5)
	{
		while(!(PIND & (1<<3))) // Garantir que a subtra��o continue, caso o bot�o se mantenha pressionado
		{
			_delay_ms(150);
			if (FreqRespiracao > 5)
				FreqRespiracao--;
			showResp3(FreqRespiracao);
		}
	}
}
ISR(TIMER0_COMPA_vect) // interrup��o do TC0 a cada 1ms = (64*(249+1))/16MHz
{
	PORTD ^= 0b00000001;
	tempo_ms++;
}
ISR(PCINT2_vect)
{
	static uint32_t catch_tempo; // vari�vel para capturar o tempo anterior
	
	if(!(PIND & (1<<1)))
	{
		bpm = (1000*60/(tempo_ms - catch_tempo));
		catch_tempo = tempo_ms; // "Capturando" o tempo atual
	}
}
int main(void)
{
	DDRD  = 0b00000001; // PD1..4 = ENTRADA, PD0 = SA�DA - alt. 0x00 sem &
	PORTD = 0b00011110; // Habilita os resistores de pull-up das portas PD1..4 - alt. 0xFF sem |
	DDRB  = 0b11111111; // PB0..7 = SA�DA
	
	// Configura��o das interrup��es
	EICRA = 0b00001010;// interrup��es externas INT0 e INT1 na borda de descida
	EIMSK = 0b00000011;// habilita as interrup��es externas INT0 e INT1
	
	// Configura��o do Timer de 1 ms
	TCCR0A = 0b00000010; // habilita modo CTC do TC0
	TCCR0B = 0b00000011; // liga TC0 com prescaler = 64
	OCR0A  = 249;		 // ajusta o comparador para o TC0 contar at� 249
	TIMSK0 = 0b00000010; // habilita a interrup��o na igualdade de compara��o com OCR0A. A interrup��o ocorre a cada 1ms = (64*(249+1))/16MHz
	
	sei();// habilita interrup��es globais, ativando o bit I do SREG
	
	while
	(1)
	{
		showResp3(FreqRespiracao);
		controlLED2(&FreqRespiracao);
	}
}

void controlLED2 (uint8_t *freq)
{
	for (int i = 0; i <= 7; i++)
	{
		PORTB |= 1<<i;
		mydelay(60000/((*freq)*16));
	}
	for (int j = 7; j >= 0; j--)
	{
		PORTB &= ~(1<<j);
		mydelay(60000/((*freq)*16));
	}
}
void showResp3(uint8_t resp)
{
	nokia_lcd_init(); // Inicia o LCD
	nokia_lcd_clear(); // Limpa o LCD
	nokia_lcd_set_cursor(0, 0); // Muda o cursor para a posi��o 0,10 ou seja, pula uma linha
	nokia_lcd_write_num(resp, 3); // Escreve uma vari�vel de tamanho 2
	nokia_lcd_set_cursor(35, 14); // Muda o cursor para a posi��o no argumento
	nokia_lcd_write_string("resp/min", 1);// Escreve um texto do tamanho 1
	nokia_lcd_set_cursor(0, 27);
	nokia_lcd_write_num(bpm, 3);
	nokia_lcd_set_cursor(51, 41);
	nokia_lcd_write_string("bpm", 1);
	nokia_lcd_render(); // Atualiza a tela do display com o conte�do do buffer
}
void mydelay(uint32_t tempo)
{
	uint32_t *aux;
	aux = &tempo_ms; // vari�vel auxiliar apontando para tempo_ms
	uint32_t cont = tempo; // contador, para a varia��o de tempo desejada
	uint32_t catch_tempo = tempo_ms; // "capturar" o tempo no instante da inicializa��o da fun��o
	
	while(cont > 0)
	{
		if (catch_tempo < *aux)
			{
			catch_tempo++;
			cont--;
			}
	}
	//*aux = 0; // zerando tempo_ms para impedir overflow quando o programa passar muito tempo sendo executado.
}