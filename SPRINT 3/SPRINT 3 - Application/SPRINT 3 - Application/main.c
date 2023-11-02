/*
 * SPRINT 3 - Application.c
 *
 * Created: 17/03/2021 16:42:46
 * Author : LUAN FÁBIO MARINHO GALINDO
 *			118110382
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nokia5110.h"

void controlLED2 (uint8_t *);
void showResp2 (uint8_t );

uint8_t FreqRespiracao = 5; // Definir um valor inicial

ISR(INT0_vect) //interrupção externa 0, quando o botão é pressionado, a freq aumenta
{
	if (FreqRespiracao < 30)
	{
		while(!(PIND & (1<<2))) // Garantir que a soma continue, caso o botão se mantenha pressionado
		{
			_delay_ms(200);
			if (FreqRespiracao < 30)
				FreqRespiracao++;
			showResp2(FreqRespiracao);
		}	
	}
}
ISR(INT1_vect) //interrupção externa 1, quando o botão é pressionado, a freq diminui
{
	if (FreqRespiracao > 5)
	{
		while(!(PIND & (1<<3))) // Garantir que a subtração continue, caso o botão se mantenha pressionado
		{
			_delay_ms(200);
			if (FreqRespiracao > 5)
				FreqRespiracao--;
			showResp2(FreqRespiracao);		
		}	
	}
}

int main(void)
{	
	DDRD &= 0b00000; // PD0..4 = ENTRADA - alt. 0x00 sem &
	PORTD |= 0b11111; // Habilita os resistores de pull-up das portas PD0..4 - alt. 0xFF sem |
	DDRB |= 0b11111111; // PB0..7 = SAÍDA
	
	// Configuração das interrupções
	EICRA = 0b00001010;// interrupções externas INT0 e INT1 na borda de descida
	EIMSK = 0b00000011;// habilita as interrupções externas INT0 e INT1
	sei();// habilita interrupções globais, ativando o bit I do SREG
	
	while
	(1)
	{
		showResp2(FreqRespiracao);
		controlLED2(&FreqRespiracao);		
	}
}

void controlLED2 (uint8_t *freq)
{
	for (int i = 0; i <= 7; i++)
	{
		PORTB |= 1<<i;
		_delay_ms(60000/((*freq)*16));
	}
	for (int j = 7; j >= 0; j--)
	{
		PORTB &= ~(1<<j);
		_delay_ms(60000/((*freq)*16));
	}
}
void showResp2(uint8_t resp)
{
	nokia_lcd_init(); // Inicia o LCD
	nokia_lcd_clear(); // Limpa o LCD
	nokia_lcd_set_cursor(0, 10); // Muda o cursor para a posição 0,10 ou seja, pula uma linha
	nokia_lcd_write_num(resp, 4); // Escreve uma variável de tamanho 4
	nokia_lcd_set_cursor(30, 40); // Muda o cursor para a posição 30,40
	nokia_lcd_write_string("resp/min", 1);// Escreve um texto do tamanho 1
	nokia_lcd_render(); // Atualiza a tela do display com o conteúdo do buffer
}