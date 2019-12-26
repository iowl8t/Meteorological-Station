#include "usart.h"
#include <util/atomic.h>

/* Якщо швидкість подвоєна (UX2=1), ділити F_CPU на 8, інакше на 16 */
#define USART_BAUDVALUE	(((F_CPU / (USART_BAUDRATE * 8UL))) - 1)
//#define USART_BAUDVALUE ((((F_CPU / 8) + (USART_BAUDRATE / 2)) / (USART_BAUDRATE)) - 1)

/* Ініціалізація */
void USART_Init(uint16_t speed)
{
	#ifdef USART_BAUDRATE
	 UBRRH = (uint8_t) (USART_BAUDVALUE>>8);
	 UBRRL = (uint8_t) USART_BAUDVALUE;
	#else
	 UBRRH = (uint8_t) (speed>>8);
	 UBRRL = (uint8_t) speed;
	 #endif
	UCSRB |= (1<<RXEN)|(1<<TXEN); //вмикаємо usart
	UCSRB |= (1<<RXCIE); //переривання по прийому
	UCSRA |= (1<<U2X); //для 8 МГц
	// Звертаємося саме до регістру UCSRC(URSEL=1)
	UCSRC |= (1<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
	//асинхронний режим(UMSEL=0), без контролю парності UPM1=0 і UPM0=0
	//1 стоп-біт(USBS=0), 8-бітна посилка(UCSZ1=1 і UCSZ0=1)
	//UCSRC |= (1<<UPM1); //парність
}

/**********************************************************************************/
/******************************* ПЕРЕДАЧА ДАНИХ ***********************************/
/**********************************************************************************/

/* Кільцевий буфер передавача */
static volatile uint8_t tBuff[USART_BUF_SIZE];
static volatile uint8_t tCount = 0, tTail = 0, tHead = 0;



/* Надсилає байт без переривання */
void USART_PutByteNoInterrupt(const uint8_t ch)
{
	while(!(UCSRA & (1<<UDRE)));
	UDR = ch;
}

/* Надсилає байт */
void USART_PutByte(const uint8_t ch)
{	
	while(tCount == USART_BUF_SIZE);
	
	if(tCount < USART_BUF_SIZE)
	{
		tBuff[tTail++] = ch;
		tCount++;
		tTail &= (USART_BUF_SIZE - 1);
		UCSRB |= (1<<UDRIE);
	}
}

/* Записує байт в буфер передавача */
static void USART_PutByteInBuffer(const uint8_t ch)
{
	while(tCount == USART_BUF_SIZE);
	
	//if(tCount < USART_BUF_SIZE)
	{
		tBuff[tTail++] = ch;
		tCount++;
		tTail &= (USART_BUF_SIZE - 1);
		UCSRB |= (1<<UDRIE);
	}
}

/* Записує рядок в буфер передавача */
void USART_PutStr(const char *str)
{	
	uint8_t ch = 0;
	while(*str)
	{
		ch = *str;
		USART_PutByteInBuffer(ch);
		str++;
	}
}

/* Передає байт */
ISR(USART_UDRE_vect)
{
	if(tCount > 0){
		UDR = tBuff[tHead++];
		tCount--;
		tHead &= USART_BUF_SIZE-1;
	}

	if(tCount == 0) UCSRB &= ~(1<<UDRIE);
}

/* Очищення Буфера передавача */
void USART_FlashTBuff(void)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		tTail	= 0;
		tCount	= 0;
		tHead	= 0;
	}
}

/**********************************************************************************/
/******************************* ПРИЙОМ ДАНИХ ***********************************/
/**********************************************************************************/


/* Кільцевий буфер приймача */
static volatile uint8_t rBuff[USART_BUF_SIZE];
static volatile uint8_t rCount = 0, rTail = 0, rHead = 0;

/* Приймання та запис в буфер байту */
ISR(USART_RXC_vect)
{
	if (rCount <= USART_BUF_SIZE)
	{
		rBuff[rTail] = UDR;
		rTail++;
		rCount++;
		rTail &= (USART_BUF_SIZE - 1);
	}
}

/* Отримання байту з буфера */
uint8_t USART_GetByte(void)
{
	if(rCount > 0)
	{
		uint8_t ch = rBuff[rHead];
		rHead++;
		rCount--;
		rHead &= (USART_BUF_SIZE - 1);		
		return ch;	
	}
	else return 0;
}