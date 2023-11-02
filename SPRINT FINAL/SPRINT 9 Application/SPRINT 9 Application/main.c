/*
 * SPRINT 9 Application.c
 *
 * Created: 21/05/2021 11:05:15
 * Author : LUAN F�BIO MARINHO GALINDO
 *			118110382
 */

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/(16*BAUD) - 1 // c�lculo de UBRR para o modo normal ass�ncrono

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "nokia5110.h"

void showResp_O2 (void);
void showHP(void);
void mydelay (uint32_t);
void mudacanal(void);
void USART_Init(unsigned int);
void USART_Transmit(unsigned char);
unsigned char USART_Receive(void);
void USART_pressao(unsigned char);
void controlSlaveBVM(uint8_t *);
void controlValve(uint8_t *);

uint8_t FreqRespiracao = 5; // Definindo valores iniciais
uint8_t satO2 = 70;
uint8_t valve_O2 = 0;
uint8_t flag_select = 0;   // Flag do seletor; vai variar entre 0 e 2
uint8_t vol_O2 = 0;
uint32_t bpm = 80;
static uint32_t tempo_ms;
float temp_C = 36.0;
unsigned char pressao[8] = "120x80";

ISR(INT0_vect) // interrup��o externa 0, quando o bot�o � pressionado, a freq aumenta || a v�lvula de O2 aumenta || o volume de O2 aumenta
{
	if (flag_select == 1)
	{
		if (FreqRespiracao < 30)
		{
			while(!(PIND & (1<<2))) // Garantir que a soma continue, caso o bot�o se mantenha pressionado
			{
				_delay_ms(150);
				if (FreqRespiracao < 30)
				FreqRespiracao++;
				showResp_O2();
			}
		}
	}
	else if (flag_select == 2)
	{
		if (valve_O2 < 100)
		{
			while(!(PIND & (1<<2))) // Garantir que a soma continue, caso o bot�o se mantenha pressionado
			{
				_delay_ms(150);
				if (valve_O2 < 100)
				valve_O2 +=10;
				showResp_O2();
				controlValve(&valve_O2);
			}
		}
	}
	else if (flag_select == 3)
	{
		if (vol_O2 < 8)
		{
			while(!(PIND & (1<<2))) // Garantir que a soma continue, caso o bot�o se mantenha pressionado
			{
				_delay_ms(150);
				if (vol_O2 < 8)
				vol_O2++;
				showResp_O2();
			}
		}
	}
	
}
ISR(INT1_vect) // interrup��o externa 1, quando o bot�o � pressionado, a freq diminui || a v�lvula de O2 diminui || o volume de O2 aumenta
{
	if (flag_select == 1)
	{
		if (FreqRespiracao > 5)
		{
			while(!(PIND & (1<<3))) // Garantir que a subtra��o continue, caso o bot�o se mantenha pressionado
			{
				_delay_ms(150);
				if (FreqRespiracao > 5)
				FreqRespiracao--;
				showResp_O2();
			}
		}
	}
	else if (flag_select == 2)
	{
		if (valve_O2 > 0)
		{
			while(!(PIND & (1<<3))) // Garantir que a subtra��o continue, caso o bot�o se mantenha pressionado
			{
				_delay_ms(150);
				if (valve_O2 > 0)
				valve_O2 -=10;
				showResp_O2();
				controlValve(&valve_O2);
			}
		}
	}
	else if (flag_select == 3)
	{
		if (vol_O2 > 0)
		{
			while(!(PIND & (1<<3))) // Garantir que a subtra��o continue, caso o bot�o se mantenha pressionado
			{
				_delay_ms(150);
				if (vol_O2 > 0)
				vol_O2--;
				showResp_O2();
			}
		}
	}
}
ISR(TIMER0_COMPA_vect) // interrup��o do TC0 a cada 1ms = (64*(249+1))/16MHz
{
	tempo_ms++;
	if ((tempo_ms % 150) == 0) // mudan�a de canal a cada 150ms
		mudacanal();
	if((tempo_ms % 200) == 0) // mostrando os dados no LCD a cada 200ms
	{
		if (flag_select == 0)
			showHP();
		else
			showResp_O2();
	}
	if ((tempo_ms % 5000) == 0)
		nokia_lcd_init(); // Reiniciando o LCD a cada 5s
}
ISR(PCINT2_vect) // Contador de BPM
{
	static uint32_t catch_tempo; // vari�vel para capturar o tempo anterior
	
	bpm = (60000.0)/((tempo_ms - catch_tempo)*2.0); // *2.0 porque a interrup��o ocorre a cada T/2 (subida e descida) | freq(hertz) = freq(BPM)/60
	catch_tempo = tempo_ms; // "Capturando" o tempo atual
}
ISR(USART_RX_vect) // Controle da entrada serial da press�o
{
	static uint8_t flag_UART;
	unsigned char recebido = USART_Receive(); // Esse recebido � para n�o chamar a fun��o de receber sempre
	
	if (recebido == ';')
	flag_UART = 1; // Dado in�cio ao recebimento
	else if (recebido == ':')
	{
		flag_UART = 0;				       // Finalizado o recebimento
		USART_pressao(recebido);           // enviando ":" para a fun��o de press�o
	}
	else
	{
		if (flag_UART == 1)
		USART_pressao(recebido);
		else
		sprintf(pressao, "ERRO!"); // Imprimindo "ERRO!" em press�o, caso chegue um recebido e a flag esteja em zero
	}
}
ISR(PCINT0_vect) // Controle do bot�o de sele��o
{
	if (!(PINB & (1<<6)))
	flag_select++;
	
	if (flag_select == 0)
	showHP();
	
	else
	showResp_O2();

	if (flag_select > 3)
	flag_select = 0;
}
int main(void)
{
	DDRD  = 0b10000000; // PD0..6 - ENTRADA, PD7 - SA�DA
	PORTD = 0b00011100; // Habilita os resistores de pull-up das portas PD2, PD3
	DDRB  = 0b00000110; // PB1,2 = SA�DA DOS SERVOS
	PORTB = 0b01000000; // Habilita o pull-up de PB6
	DDRC  = 0b01111100; // sa�da do LCD PC2...PC6; PC0 e PC1 - Entradas das fontes vari�veis
	PORTC = 0b00000000; // Desabilitando os pull-ups da porta
	
	// Configura��o das interrup��es
	EICRA  = 0b00001010; // interrup��es externas INT0 e INT1 na borda de descida
	EIMSK  = 0b00000011; // habilita as interrup��es externas INT0 e INT1
	PCICR  = 0b00000101; // interrup��es pin change 2 (porta D) e pin change 0 (Porta B)
	PCMSK2 = 0b00010000; // interrup��o pin change PD4 - Contador de BPM
	PCMSK0 = 0b01000000; // interrup��o pin change PB6 - Chave Seletora
	
	// Configura��o do Timer de 1 ms
	TCCR0A = 0b00000010; // habilita modo CTC do TC0
	TCCR0B = 0b00000011; // liga TC0 com prescaler = 64
	OCR0A  = 249;		 // ajusta o comparador para o TC0 contar at� 249
	TIMSK0 = 0b00000010; // habilita a interrup��o na igualdade de compara��o com OCR0A. A interrup��o ocorre a cada 1ms = (64*(249+1))/16MHz
	
	// Configura ADC
	ADCSRA  = 0b11100111;    // habilita o AD, habilita interrup��o, modo de convers�o cont�nua, prescaler = 128
	ADCSRB  = 0b00000000;    // modo de convers�o cont�nua
	// medi��o inicial
	ADMUX   = 0b01000000;    // Tens�o interna de ref VCC, canal 0
	DIDR0   = 0b00111110;    // habilita pino PC0 como entrada de ADC0
	temp_C  = 10.0*(5.0*ADC/1023.0 + 1.0);
	
	// Fast PWM, Timer TC1, OC1A e OC1B habilitados
	ICR1   = 39999; // Per�odo do PWM - TOP = F_CPU/(freq*prescaler) - 1 = 16M/(50*8) - 1 -> T = 20ms
	TCCR1A = 0b10100010; // PWM n�o invertido nos pinos OC1A e OC1B, via ICR1
	TCCR1B = 0b00011010; // liga TC1, prescaler = 8
	// variando de 2000 a 4000, para 0� a 180�, sendo inicializados em 0�
	OCR1A  = 2000; // Servo BVM
	OCR1B  = 2000; // servo O2
	
	sei(); // habilita interrup��es globais, ativando o bit I do SREG
	
	USART_Init(MYUBRR); // Inicializando a USART
	
	nokia_lcd_init(); // Iniciando o LCD
	
	while (1)
	{
		controlSlaveBVM(&FreqRespiracao);
	}
}
// Configura��o do display modo 1
void showResp_O2(void)
{
	nokia_lcd_clear(); // Limpa o LCD
	
	// In�cio
	nokia_lcd_set_cursor(0, 0);
	nokia_lcd_write_string("Parametros", 1);
	nokia_lcd_set_cursor(0, 7);
	nokia_lcd_write_string("~~~~~~~~~~~~~~", 1);
	

	// Frequ�ncia de Respira��o
	nokia_lcd_set_cursor(0, 15); // Muda o cursor para a posi��o 0,0
	nokia_lcd_write_int(FreqRespiracao, 1); // Escreve uma vari�vel de tamanho 1
	nokia_lcd_set_cursor(35, 15); // Muda o cursor para a posi��o no argumento
	nokia_lcd_write_string("resp/min", 1);// Escreve um texto do tamanho 1
	
	// V�lvula de O2
	nokia_lcd_set_cursor(0, 25);
	nokia_lcd_write_int(valve_O2, 1);
	nokia_lcd_set_cursor(35, 25);
	nokia_lcd_write_string("%O2", 1);
	
	// Volume de O2
	nokia_lcd_set_cursor(0, 35);
	nokia_lcd_write_int(vol_O2, 1);
	nokia_lcd_set_cursor(35, 35);
	nokia_lcd_write_string("vol. O2", 1);
	
	// Indicador de sele��o
	if (flag_select == 1)
	{
		nokia_lcd_set_cursor(20, 15);
		nokia_lcd_write_string("->", 1);
	}
	else if (flag_select == 2)
	{
		nokia_lcd_set_cursor(20, 25);
		nokia_lcd_write_string("->", 1);
	}
	else
	{
		nokia_lcd_set_cursor(20, 35);
		nokia_lcd_write_string("->", 1);
	}
	nokia_lcd_render(); // Atualiza a tela do display com o conte�do do buffer
}
// Configura��o do display modo 2
void showHP (void)
{
	nokia_lcd_clear(); // Limpa o LCD
	
	// In�cio
	nokia_lcd_set_cursor(0, 0);
	nokia_lcd_write_string("Sinais Vitais", 1);
	
	//	Frequ�ncia Card�aca
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_int(bpm, 1);
	nokia_lcd_set_cursor(45, 10);
	nokia_lcd_write_string("bpm", 1);
	
	// Satura��o de O2 no sangue
	nokia_lcd_set_cursor(0, 20);   // As coordenadas das novas vari�veis escritas ir�o tomar como base
	nokia_lcd_write_int(satO2, 1); // a diferen�a entre as coordenadas
	nokia_lcd_set_cursor(45, 20);
	nokia_lcd_write_string("% SpO2", 1);
	
	// Temperatura Corporal
	nokia_lcd_set_cursor(0, 30);
	nokia_lcd_write_float(temp_C, 1);
	nokia_lcd_set_cursor(45, 30);
	nokia_lcd_write_string("�C", 1);
	
	// Press�o Sangu�nea
	nokia_lcd_set_cursor(0, 40);
	nokia_lcd_write_string(pressao, 1);
	nokia_lcd_set_cursor(45, 40);
	nokia_lcd_write_string("mmHg", 1);
	
	nokia_lcd_render(); // Atualiza a tela do display com o conte�do do buffer
}
// Delay Personalizado
void mydelay(uint32_t tempo)
{
	uint32_t *aux;
	aux = &tempo_ms;                 // vari�vel auxiliar apontando para tempo_ms
	uint32_t cont = tempo;           // contador, para a varia��o de tempo desejada
	uint32_t catch_tempo = tempo_ms; // "capturar" o tempo no instante da inicializa��o da fun��o
	
	while(cont > 0) // *aux <= tempo + catch_tempo - alternativa
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
// Controle da temperetura corporal e da satura��o de O2 no sangue
void mudacanal(void)
{
	if (ADMUX == 0b01000000)
	{
		ADMUX = 0b01000001;       // Tens�o interna de ref VCC, canal 1
		DIDR0 = 0b00111101;       // habilita pino PC1 como entrada de ADC1
		satO2 = ADC*125.0/1023.0; // A fonte, por algum motivo, s� toma 5V como valor de refer�ncia.
		//		satO2 = ADC;
	}
	else
	{
		ADMUX   = 0b01000000;
		DIDR0   = 0b00111110;
		temp_C  = 10.0*((5.0*ADC)/1023.0 + 1.0); // A fonte, por algum motivo, s� toma 5V como valor de refer�ncia.
		//		temp_C = ADC;
	}

	if (temp_C < 35 || temp_C > 41 || satO2 < 60)
	PORTD |= 0b10000000;
	
	else if (temp_C >= 35 && temp_C <= 41 && satO2 >= 60)
	PORTD &= 0b01111111;
}
// Fun��o para inicializa��o da USART
void USART_Init(unsigned int ubrr)
{
	UBRR0H = (unsigned char)(ubrr>>8);          // Ajusta a taxa de transmiss�o - 8bits >>
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); // Habilita o transmissor e o receptor
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);            // Ajusta o formato do frame: 8 bits de dados e 1 de parada
}
// Fun��o para envio de um frame de 5 a 8bits
void USART_Transmit(unsigned char data)
{
	while(!(UCSR0A & (1<<UDRE0)));      // Espera a limpeza do registr. de transmiss�o
	UDR0 = data;                        // Coloca o dado no registrador e o envia
}
// Fun��o para recep��o de um frame de 5 a 8bits
unsigned char USART_Receive(void)
{
	while(!(UCSR0A & (1<<RXC0)));       // Espera o dado ser recebido
	return UDR0;                        // L� o dado recebido e retorna
}
// Controle da press�o arterial
void USART_pressao(unsigned char recebido)
{
	static uint8_t cont; // Contador para o tamanho da palavra
	static unsigned char aux[8];
	if (recebido != ':')
	{
		aux[cont] = recebido;
		USART_Transmit(aux[cont]);
		cont++;
	}
	else if ((recebido == ':') && (cont == 7))
	{
		cont = 0;						 // Tamanho da palavra atingido. Reiniciando palavra
		sprintf(pressao, aux);
		sprintf(aux, "reset. ");		 // Resetando o auxiliar para evitar overflow
	}
	else if ((recebido == ':') && (cont == 6)) // Caso de a palavra vir no formato HHHxMM, e.g. 150x80
	{
		cont = 0;
		sprintf(pressao, aux);
		sprintf(aux, "reset. ");
	}
	else if ((recebido == ':') && (cont > 7))
	{
		cont = 0;
		sprintf(pressao, "ERRO!");
		sprintf(aux, "reset. ");
	}
	else if ((recebido == ':') && (cont < 6))
	{
		cont = 0;
		sprintf(pressao, "ERRO!");
		sprintf(aux, "reset. ");
	}
}
// Controle do servo  BVM
void controlSlaveBVM(uint8_t *freq)
{
	uint16_t posicao[8] = {2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000}; // Pelo Teorema de Tales, posi��o[angulo] = (200*angulo + 36000)/18
	uint8_t i;
	
	if (vol_O2 == 0)
	OCR1A = 2000;
	else
	{
		for (i = 0; i < vol_O2; i++)
		{
			OCR1A = posicao[i]; // Servo BVM
			mydelay(60000.0/((*freq)*16.0)*(8.0/vol_O2));
			if (OCR1A == 2000)
			{
				PORTD |= 0b10000000;
				_delay_ms(500);
				PORTD &= 0b01111111;
			}
		}
		for (i = vol_O2; i > 0; i--)
		{
			OCR1A = posicao[i];
			mydelay(60000.0/((*freq)*16.0)*(8.0/vol_O2));
		}
	}
}
// Controle do servo O2
void controlValve(uint8_t *valve)
{
	OCR1B = 20*(*valve) + 2000; // do Teorema de Tales, N = 20*porcentagem + 2000
}