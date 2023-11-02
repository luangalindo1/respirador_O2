/*
 * SPRINT 5 - Application.c
 *
 * Created: 07/04/2021 15:25:24
 * Author : LUAN FÁBIO MARINHO GALINDO
 *			118110382
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nokia5110.h"

void controlLED (uint8_t *);
void showResp (void);
void mydelay (uint32_t);
void mudacanal(void);
uint8_t FreqRespiracao = 5; // Definir um valor inicial
uint32_t tempo_ms = 0;
uint8_t bpm = 0;
uint8_t satO2 = 50;
uint8_t temp_C = 30;

ISR(INT0_vect) // interrupção externa 0, quando o botão é pressionado, a freq aumenta
{
	if (FreqRespiracao < 30)
	{
		while(!(PIND & (1<<2))) // Garantir que a soma continue, caso o botão se mantenha pressionado
		{
			_delay_ms(150);
			if (FreqRespiracao < 30)
			FreqRespiracao++;
			showResp(); // A função deve ser incluída aqui para a mudança de resp/min seja mostrada imediatamente no display
		}
	}
}
ISR(INT1_vect) // interrupção externa 1, quando o botão é pressionado, a freq diminui
{
	if (FreqRespiracao > 5)
	{
		while(!(PIND & (1<<3))) // Garantir que a subtração continue, caso o botão se mantenha pressionado
		{
			_delay_ms(150);
			if (FreqRespiracao > 5)
			FreqRespiracao--;
			showResp();
		}
	}
}
ISR(TIMER0_COMPA_vect) // interrupção do TC0 a cada 1ms = (64*(249+1))/16MHz
{
	PORTD ^= 0b00000001;
	tempo_ms++;
	if ((tempo_ms % 150) == 0) // mudança de canal a cada 150ms
		mudacanal();
	if((tempo_ms % 200) == 0) // mostrando os dados no LCD a cada 200ms
		showResp();	
}
ISR(PCINT2_vect)
{
	static uint32_t catch_tempo; // variável para capturar o tempo anterior
	
	bpm = (1000.0*60.0)/((tempo_ms - catch_tempo)*2.0); // *2 porque a interrupção ocorre a cada T/2 (subida e descida)
	if (catch_tempo < tempo_ms)
		catch_tempo = tempo_ms; // "Capturando" o tempo atual
	
}
int main(void)
{
	DDRD  = 0b10000001; // PD1..4 = ENTRADA, PD0,PD7 = SAÍDA
	PORTD = 0b00001100; // Habilita os resistores de pull-up das portas PD2 E PD3
	DDRB  = 0b11111111; // PB0..7 = SAÍDA
	DDRC  = 0b01111100; // saída do LCD PC2...PC6; PC0 e PC1 - Entradas das fontes variáveis
	PORTC = 0b00000000; // Desabilitando os pull-ups da porta
	
	// Configuração das interrupções
	EICRA  = 0b00001010; // interrupções externas INT0 e INT1 na borda de descida
	EIMSK  = 0b00000011; // habilita as interrupções externas INT0 e INT1
	PCICR  = 0b00000100; // interrupções pin change 2 (porta D)
	PCMSK2 = 0b00000010; // interrupções pin change PD1 - Contador de BPM
	
	// Configuração do Timer de 1 ms
	TCCR0A = 0b00000010; // habilita modo CTC do TC0
	TCCR0B = 0b00000011; // liga TC0 com prescaler = 64
	OCR0A  = 249;		 // ajusta o comparador para o TC0 contar até 249
	TIMSK0 = 0b00000010; // habilita a interrupção na igualdade de comparação com OCR0A. A interrupção ocorre a cada 1ms = (64*(249+1))/16MHz
	
	// Configura ADC
	ADCSRA  = 0b11100111;    // habilita o AD, habilita interrupção, modo de conversão contínua, prescaler = 128
	ADCSRB  = 0b00000000;    // modo de conversão contínua
	// medição inicial
	ADMUX   = 0b01000000;    // Tensão interna de ref VCC, canal 0
	DIDR0   = 0b00111110;    // habilita pino PC0 como entrada de ADC0
	temp_C  = 10.0*(5.0*ADC/1023.0 + 1.0);
	
	sei(); // habilita interrupções globais, ativando o bit I do SREG
	
	while
	(1)
	{
		nokia_lcd_init(); // Inicia o LCD
		controlLED(&FreqRespiracao);
	}
}
void controlLED (uint8_t *freq)
{
	for (int i = 0; i <= 7; i++)
	{
		PORTB |= 1<<i;
		mydelay(60000.0/((*freq)*16.0));
	}
	for (int j = 7; j >= 0; j--)
	{
		PORTB &= ~(1<<j);
		mydelay(60000.0/((*freq)*16.0));
	}
}
void showResp(void)
{
	nokia_lcd_clear(); // Limpa o LCD
	nokia_lcd_set_cursor(0, 0); // Muda o cursor para a posição 0,0
	nokia_lcd_write_num(FreqRespiracao, 1); // Escreve uma variável de tamanho 1
	nokia_lcd_set_cursor(35, 1); // Muda o cursor para a posição no argumento
	nokia_lcd_write_string("resp/min", 1);// Escreve um texto do tamanho 1
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_num(bpm, 1);
	nokia_lcd_set_cursor(35, 10);
	nokia_lcd_write_string("bpm", 1);
	nokia_lcd_set_cursor(0, 20);   // As coordenadas das novas variáveis escritas irão tomar como base
	nokia_lcd_write_num(satO2, 1); // a diferença entre as coordenadas resp/min e bpm 
	nokia_lcd_set_cursor(35, 20);
	nokia_lcd_write_string("% SpO2", 1);
	nokia_lcd_set_cursor(0, 30);
	nokia_lcd_write_num(temp_C, 1);
	nokia_lcd_set_cursor(35, 30);
	nokia_lcd_write_string("ºC", 1);
	nokia_lcd_render(); // Atualiza a tela do display com o conteúdo do buffer
}
void mydelay(uint32_t tempo)
{
	uint32_t *aux;
	aux = &tempo_ms; // variável auxiliar apontando para tempo_ms
	uint32_t cont = tempo; // contador, para a variação de tempo desejada
	uint32_t catch_tempo = tempo_ms; // "capturar" o tempo no instante da inicialização da função
	
	while(cont > 0)
	{
		if (catch_tempo < *aux)
		{
			catch_tempo++;
			cont--;
		}
	}
	if(*aux >= 4294967295) // 2^32 - 1 = 4294967295 
		*aux = 0; // zerando tempo_ms para impedir overflow quando o programa passar muito tempo sendo executado
}
void mudacanal(void)
{
		if (ADMUX == 0b01000000)
		{
			ADMUX = 0b01000001; // Tensão interna de ref VCC, canal 1
			DIDR0 = 0b00111101; // habilita pino PC1 como entrada de ADC1
			satO2 = ADC*125.0/1023.0; // A fonte, por algum motivo, só toma 5V como valor de referência.
//			satO2 = ADC;
		}
		else
		{
			ADMUX   = 0b01000000;
			DIDR0   = 0b00111110;
			temp_C  = 10.0*((5.0*ADC)/1023.0 + 1.0); // A fonte, por algum motivo, só toma 5V como valor de referência.
//			temp_C = ADC;
		}
		if (temp_C < 35 || temp_C > 41 || satO2 < 60)
			PORTD |= 0b10000000;
			
		else if (temp_C >= 35 && temp_C <= 41 && satO2 >= 60)
			PORTD &= 0b01111111;
}